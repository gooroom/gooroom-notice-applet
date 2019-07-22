/*
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

#ifndef __GOOROOM_NOTICE_APPLET_H__
#define __GOOROOM_NOTICE_APPLET_H__

#include <panel-applet.h>
G_BEGIN_DECLS

#define GOOROOM_TYPE_NOTICE_APPLET           (gooroom_notice_applet_get_type ())
#define GOOROOM_NOTICE_APPLET(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOOROOM_TYPE_NOTICE_APPLET, GooroomNoticeApplet))
#define GOOROOM_NOTICE_APPLET_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST    ((obj), GOOROOM_TYPE_NOTICE_APPLET, GooroomNoticeAppletClass))
#define GOOROOM_IS_NOTICE_APPLET(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOOROOM_TYPE_NOTICE_APPLET))
#define GOOROOM_IS_NOTICE_APPLET_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE    ((obj), GOOROOM_TYPE_NOTICE_APPLET))
#define GOOROOM_NOTICE_APPLET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS  ((obj), GOOROOM_TYPE_NOTICE_APPLET, GooroomNoticeAppletClass))

typedef struct _GooroomNoticeApplet        GooroomNoticeApplet;
typedef struct _GooroomNoticeAppletClass   GooroomNoticeAppletClass;
typedef struct _GooroomNoticeAppletPrivate GooroomNoticeAppletPrivate;

struct _GooroomNoticeApplet {
	PanelApplet            parent;
	GooroomNoticeAppletPrivate *priv;
};

struct _GooroomNoticeAppletClass {
	PanelAppletClass parent_class;
};

GType gooroom_notice_applet_get_type (void);
gboolean gooroom_notice_applet_job (gpointer user_data);
void gooroom_applet_notice_get_data_from_json (gpointer user_data, const gchar *data, gboolean urgency);
void gooroom_log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data);

G_END_DECLS

#endif /* __GOOROOM_NOTICE_APPLET_H__*/
