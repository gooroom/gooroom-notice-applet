/*
 * Copyright (C) 2015-2017 Gooroom <gooroom@gooroom.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gooroom-notice-applet.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <panel-applet.h>

#include <libnotify/notify.h>
#include <webkit2/webkit2.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#define PANEL_TRAY_ICON_SIZE        (22)
#define NOTIFICATION_LIMIT          (5)

struct _GooroomNoticeAppletPrivate
{
	GtkWidget  *tray;
	GtkWidget  *button;
    GtkWidget  *window;   
	gint        panel_size;
    int         total;
    gboolean    img_status;
    GQueue     *queue;
    GHashTable *data_list;
};

struct agent_data
{
    gchar    *title;
    gchar    *msg;
    gchar    *user_id;
    gchar    *token;
    gchar    *session_id;
    gchar    *url;
};

struct view_data
{
    gchar    *user_id;
    gchar    *token;
    gchar    *session_id;
    gchar    *url;
};

G_DEFINE_TYPE_WITH_PRIVATE (GooroomNoticeApplet, gooroom_notice_applet, PANEL_TYPE_APPLET)

static gboolean    is_job = FALSE;

static gboolean 
gooroom_test_icon_change_job (gpointer user_data)
{
    GooroomNoticeAppletPrivate *priv = (GooroomNoticeAppletPrivate *)user_data;

	gtk_container_remove (GTK_CONTAINER (priv->button), priv->tray);
   
    gchar *icon = NULL;
    
    if (priv->img_status)
        icon = g_strdup("face-sad");
    else
        icon = g_strdup("face-smile");

    GdkPixbuf *pix;
	pix = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
			icon,
			PANEL_TRAY_ICON_SIZE,
			GTK_ICON_LOOKUP_FORCE_SIZE, NULL);

    g_free (icon);
   
    if (pix)
    {
        priv->tray = gtk_image_new_from_pixbuf (pix);
	    gtk_image_set_pixel_size (GTK_IMAGE (priv->tray), PANEL_TRAY_ICON_SIZE);
	    gtk_container_add (GTK_CONTAINER (priv->button), priv->tray);
	    g_object_unref (G_OBJECT (pix));
    } 
    gtk_widget_show_all (priv->button);
    
    priv->img_status = !priv->img_status;
    
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//Gooroom Agent
////////////////////////////////////////////////////////////////////////////////////////////////
static GDBusProxy *agent_proxy = NULL;

static void
agent_signal_cb (GDBusProxy *proxy,
                 gchar *sender_name,
                 gchar *signal_name,
                 GVariant *parameters,
                 gpointer user_data)
{
    g_return_if_fail (user_data != NULL);

    GVariant *v;
    g_variant_get (parameters, "(v)", &v);
    gchar *res = g_variant_dup_string (v, NULL);

    g_printerr ("[signal name : %s]\n", signal_name);
}

static GDBusProxy *
agent_proxy_get (void)
{
    if (agent_proxy == NULL)
    {
        agent_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                G_DBUS_CALL_FLAGS_NONE,
                NULL,
                "kr.gooroom.agent",
                "/kr/gooroom/agent",
                "kr.gooroom.agent",
                NULL,
                NULL);
    }

    return agent_proxy;
}

static void
gooroom_agent_bind_signal (gpointer data)
{
    agent_proxy = agent_proxy_get();
    if (agent_proxy) 
    {
        g_signal_connect (agent_proxy, "g-signal", G_CALLBACK (agent_signal_cb), data);
    }
}

static gchar*
gooroom_application_client_id_get ()
{
    gchar *client_id = NULL;

    GKeyFile *keyfile;
    g_autofree gchar *filename;
    filename = g_build_filename ("/etc/gooroom/gooroom-client-server-register/", "gcsr.conf", NULL);
    keyfile = g_key_file_new ();
    
    if (g_key_file_load_from_file (keyfile, filename, G_KEY_FILE_NONE, NULL))
    {
        client_id = g_key_file_get_string (keyfile, "certificate", "client_name", NULL);
    }

    return client_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Notification
////////////////////////////////////////////////////////////////////////////////////////////////
static void
on_notification_closed (NotifyNotification *notification, gpointer user_data)
{
    GooroomNoticeApplet *applet = GOOROOM_NOTICE_APPLET (user_data);
    GooroomNoticeAppletPrivate *priv = applet->priv;
    priv->total--;

    if (!priv->window)
        g_hash_table_remove (priv->data_list, notification);
}

static void
on_notification_popup_cookie_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    g_printerr ("[on_notification_popup_cookie_cb]\n");
}

static gboolean 
on_notification_popup_closed (GtkWidget *widget, gpointer user_data)
{
    GooroomNoticeApplet *applet = GOOROOM_NOTICE_APPLET (user_data);
    GooroomNoticeAppletPrivate *priv = applet->priv;

    if (priv->window != NULL)
    {
        gtk_widget_destroy (priv->window);
        priv->window = NULL;
    }

    return TRUE;
}

static gboolean
on_notification_popup_webview_closed (WebKitWebView *web_view, GtkWidget *window)
{
    gtk_widget_destroy (window);
    return TRUE;
}

static void
on_notification_popup_opened (NotifyNotification *notification, const char *action, gpointer user_data)
{
    GooroomNoticeApplet *applet = GOOROOM_NOTICE_APPLET(user_data);
    GooroomNoticeAppletPrivate *priv = applet->priv;
    priv->img_status = !priv->img_status;

    if (priv->window != NULL)
    {
        gtk_widget_destroy (priv->window);
        priv->window = NULL;
    }

    //Data
    struct view_data *data;
    NotifyNotification *key;
    if (g_hash_table_lookup_extended (priv->data_list, (gpointer)notification, (gpointer)&key, (gpointer)&data))
    {
        g_printerr ("[UserID [%s]\n", data->user_id);
    }

    GtkWidget *window;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 5);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

    GtkWidget *main_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add (GTK_CONTAINER (window), main_vbox);
    gtk_widget_show (main_vbox);

    GtkWidget *scroll_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (main_vbox), scroll_window, TRUE, TRUE, 0);
    gtk_widget_show (scroll_window);
    
    // webkit
    WebKitWebView *view = WEBKIT_WEB_VIEW (webkit_web_view_new());
    gtk_container_add (GTK_CONTAINER (scroll_window), GTK_WIDGET(view));

    g_signal_connect (window, "destroy", G_CALLBACK (on_notification_popup_closed), (gpointer)applet);
    g_signal_connect (view, "close", G_CALLBACK (on_notification_popup_webview_closed), window);

    webkit_web_view_load_uri (view, data->url);

    // cookie
    WebKitWebContext *context = webkit_web_view_get_context (view);
    WebKitCookieManager *manager = webkit_web_context_get_cookie_manager (context);

    SoupCookie *token = soup_cookie_new ("NOTICE_ACCESS_TOKEN", data->token, "localhost", "/", 1);
    webkit_cookie_manager_add_cookie (manager,
                                      token,
                                      NULL,
                                      (GAsyncReadyCallback)on_notification_popup_cookie_cb,
                                      NULL);

    SoupCookie *user_id = soup_cookie_new ("CLIENT_ID", data->user_id, "localhost", "/", 1);
    webkit_cookie_manager_add_cookie (manager,
                                      user_id,
                                      NULL,
                                      (GAsyncReadyCallback)on_notification_popup_cookie_cb,
                                      NULL);

    gtk_widget_grab_focus (GTK_WIDGET (view));
    gtk_widget_show (GTK_WIDGET(view));

    GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end (GTK_BOX (main_vbox), hbox, FALSE, TRUE, 0);
    gtk_widget_show (hbox);

    GtkWidget *button = gtk_button_new_with_label (_("Close"));
    gtk_widget_set_can_focus (button, TRUE);
    gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_notification_popup_closed), (gpointer)applet);
    gtk_widget_show (button);

    gtk_window_set_default_size (GTK_WINDOW (window), 600, 400);
    gtk_widget_show_all (window);

    priv->window = window;

    //remove..
    g_queue_clear (priv->queue);
    g_hash_table_remove_all (priv->data_list);
}

////////////////////////////////////////////////////////////////////////////////////////////////
//Gooroom Notice Applet 
////////////////////////////////////////////////////////////////////////////////////////////////
static void
on_notice_applet_hash_key_destroy (gpointer user_data)
{
    NotifyNotification *n = (NotifyNotification*)user_data;
    notify_notification_close (n, NULL);
}

static void
on_notice_applet_hash_value_destroy (gpointer user_data)
{
    struct view_data *v = (struct view_data *)user_data;

    g_free (v->user_id);
    g_free (v->session_id);
    g_free (v->url);
}

static int cnt = 0;
static void
on_notice_applet_button_toggled (GtkToggleButton *button, gpointer user_data)
{
    GooroomNoticeApplet *applet = GOOROOM_NOTICE_APPLET (user_data);
    GooroomNoticeAppletPrivate *priv = applet->priv;

    if (priv->window != NULL)
    {
        gtk_widget_destroy (priv->window);
        priv->window = NULL;
    }

    //agent data...
    struct agent_data *data;
    data = g_try_new0 (struct agent_data, 1);
    g_autofree gchar *test_id = gooroom_application_client_id_get();
    data->title = g_strdup ("Notification");
    data->msg = g_strdup_printf ("ClientID[%s]", test_id);
    data->user_id = gooroom_application_client_id_get();
    data->session_id = NULL;
    data->token = NULL;
    data->url = g_strdup ("http://172.25.0.53:8080/gpms/notice");
    
    g_queue_push_tail (priv->queue, data);

    cnt++;

    if (!is_job)
        g_timeout_add (100, (GSourceFunc) gooroom_notice_applet_job, (gpointer)applet);
}

////////////////////////////////////////////////////////////////////////////////////////////////
//Gooroom Notice Applet 
////////////////////////////////////////////////////////////////////////////////////////////////
gboolean 
gooroom_notice_applet_job (gpointer user_data)
{
    GooroomNoticeApplet *applet = GOOROOM_NOTICE_APPLET (user_data);
    GooroomNoticeAppletPrivate *priv = applet->priv;

    is_job = TRUE;

    if (NOTIFICATION_LIMIT <= priv->total)
    {
        guint total = g_queue_get_length (priv->queue);
        if (0 == total)
            is_job = FALSE;

        return is_job;
    }
   
    struct agent_data *a;

    while ((a = g_queue_pop_head (priv->queue)))
    {
        priv->total++;

        g_autofree gchar *title = NULL;
        title = a->title;

        g_autofree gchar *msg = NULL;
        msg = a->msg;

        g_autofree gchar *icon = g_strdup ("face-smile");

        NotifyNotification *notification;
        notify_init (PACKAGE_NAME);
        notification = notify_notification_new (title, msg, icon);
        notify_notification_add_action (notification, "default", "detail view", (NotifyActionCallback)on_notification_popup_opened, applet, NULL);
        notify_notification_set_urgency (notification, NOTIFY_URGENCY_NORMAL);
        notify_notification_set_timeout (notification, NOTIFY_EXPIRES_DEFAULT);
        notify_notification_show (notification, NULL);
        
        struct view_data *v;
        v = g_try_new0 (struct view_data, 1);
        v->user_id = a->user_id;
        v->session_id = a->session_id;
        v->url = a->url;

        g_hash_table_insert (priv->data_list, notification, v);

        g_signal_connect (G_OBJECT (notification), "closed", G_CALLBACK (on_notification_closed), applet);

        if (NOTIFICATION_LIMIT <= priv->total)
            break;
    }

    guint total = g_queue_get_length (priv->queue);

    if (0 == total)
        is_job = FALSE;

    return is_job;
}

static void
gooroom_notice_applet_size_allocate (GtkWidget     *widget,
                                          GtkAllocation *allocation)
{
	gint size;
	GtkAllocation alloc;
	GtkOrientation orientation;
	GtkStyleContext *ctx;
	GtkBorder padding, border;

	GooroomNoticeApplet *applet;
	GooroomNoticeAppletPrivate *priv;

	applet = GOOROOM_NOTICE_APPLET (widget);
	priv = applet->priv;

//	PanelApplet *panel_applet = PANEL_APPLET (widget);

	GTK_WIDGET_CLASS (gooroom_notice_applet_parent_class)->size_allocate (widget, allocation);

	orientation = panel_applet_get_gtk_orientation (PANEL_APPLET (applet));

	gtk_widget_get_allocation (widget, &alloc);

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		size = alloc.height;
	else
		size = alloc.width;

	if (priv->panel_size == size)
		return;

	priv->panel_size = size;

	ctx = gtk_widget_get_style_context (GTK_WIDGET (priv->button));
	gtk_style_context_get_padding (ctx, gtk_widget_get_state_flags (GTK_WIDGET (priv->button)), &padding);
	gtk_style_context_get_border (ctx, gtk_widget_get_state_flags (GTK_WIDGET (priv->button)), &border);

	gint minus = padding.top+padding.bottom+border.top+border.bottom;

	GdkPixbuf *pix = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
			"anbox",
			size - minus,
			GTK_ICON_LOOKUP_FORCE_SIZE, NULL);

	if (pix) {
		gtk_image_set_from_pixbuf (GTK_IMAGE (priv->tray), pix);
		gtk_image_set_pixel_size (GTK_IMAGE (priv->tray), size);
		g_object_unref (G_OBJECT (pix));
	}
}

static void
gooroom_notice_applet_finalize (GObject *object)
{
	G_OBJECT_CLASS (gooroom_notice_applet_parent_class)->finalize (object);

	GooroomNoticeApplet *applet = GOOROOM_NOTICE_APPLET (object);
    GooroomNoticeAppletPrivate *priv = applet->priv;

    if (priv->window != NULL)
        gtk_widget_destroy (priv->window);

    if (priv->queue)
    {
        g_queue_clear (priv->queue);
        g_queue_free (priv->queue);

        priv->queue = NULL;
    }

    if (priv->data_list)
    {
        g_hash_table_destroy (priv->data_list);
        priv->data_list = NULL;
    }

    g_free (applet);
}

static void
gooroom_notice_applet_init (GooroomNoticeApplet *applet)
{
	GooroomNoticeAppletPrivate *priv;
	priv = applet->priv = gooroom_notice_applet_get_instance_private (applet);
	panel_applet_set_flags (PANEL_APPLET (applet), PANEL_APPLET_EXPAND_MINOR);

    priv->window     = NULL;
    priv->img_status = FALSE;

    priv->total      = 0;
    priv->queue      = g_queue_new ();
    priv->data_list  = g_hash_table_new_full (g_direct_hash, g_direct_equal, (GDestroyNotify)on_notice_applet_hash_key_destroy, (GDestroyNotify)on_notice_applet_hash_value_destroy);
    
    /* Initialize i18n */
//	setlocale (LC_ALL, "");
//	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
//	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
//	textdomain (GETTEXT_PACKAGE);

    priv->button = gtk_toggle_button_new ();
    gtk_container_add (GTK_CONTAINER (applet), priv->button);
    g_signal_connect (G_OBJECT (priv->button), "toggled", G_CALLBACK (on_notice_applet_button_toggled), applet);
    
    //agent dbus 
    gooroom_agent_bind_signal (NULL);
    g_timeout_add (1000, (GSourceFunc)gooroom_test_icon_change_job, priv);
}

static void
gooroom_notice_applet_class_init (GooroomNoticeAppletClass *class)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (class);
	widget_class = GTK_WIDGET_CLASS (class);

	object_class->finalize = gooroom_notice_applet_finalize;
}

static gboolean
gooroom_notice_applet_fill (GooroomNoticeApplet *applet)
{
    g_printerr ("[gooroom_notice_applet_fill]\n");
	g_return_val_if_fail (PANEL_IS_APPLET (applet), FALSE);

	GooroomNoticeAppletPrivate *priv = applet->priv;

	gtk_widget_show_all (GTK_WIDGET (applet));

	return TRUE;
}

static gboolean
gooroom_notice_applet_factory (PanelApplet *applet,
                                    const gchar *iid,
                                    gpointer     data)
{
    g_printerr ("[gooroom_notice_applet_factory]\n");
	gboolean retval = FALSE;

	if (!g_strcmp0 (iid, "GooroomNoticeApplet"))
		retval = gooroom_notice_applet_fill (GOOROOM_NOTICE_APPLET (applet));

	return retval;
}

PANEL_APPLET_IN_PROCESS_FACTORY ("GooroomNoticeAppletFactory",
                                 GOOROOM_TYPE_NOTICE_APPLET,
                                 (PanelAppletFactoryCallback)gooroom_notice_applet_factory,
                                 NULL)
