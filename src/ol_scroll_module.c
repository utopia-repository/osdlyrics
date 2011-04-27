#include <string.h>
#include "ol_scroll_module.h"
#include "ol_music_info.h"
#include "ol_scroll_window.h"
#include "ol_config.h"
#include "ol_color.h"
#include "ol_lrc.h"

typedef struct _OlScrollModule OlScrollModule;

struct OlLrc;

struct _OlScrollModule
{
  OlMusicInfo music_info;
  gint duration;
  struct OlLrc *lrc;
  OlScrollWindow *scroll;
};

OlScrollModule* ol_scroll_module_new (struct OlDisplayModule *module);
void ol_scroll_module_free (struct OlDisplayModule *module);
void ol_scroll_module_set_music_info (struct OlDisplayModule *module, OlMusicInfo *music_info);
void ol_scroll_module_set_played_time (struct OlDisplayModule *module, int played_time);
void ol_scroll_module_set_lrc (struct OlDisplayModule *module, struct OlLrc *lrc_file);
void ol_scroll_module_set_duration (struct OlDisplayModule *module, int duration);
/*
void ol_osd_module_search_message (struct OlDisplayModule *module, const char *message);
void ol_osd_module_search_fail_message (struct OlDisplayModule *module, const char *message);
void ol_osd_module_download_fail_message (struct OlDisplayModule *module, const char *message);
void ol_osd_module_clear_message (struct OlDisplayModule *module);*/
static void _config_change_handler (OlConfig *config,
                                    gchar *group,
                                    gchar *name,
                                    gpointer userdata);
static gboolean _window_configure_cb (GtkWidget *widget,
                                      GdkEventConfigure *event,
                                      gpointer user_data);

static gboolean
_window_configure_cb (GtkWidget *widget,
                      GdkEventConfigure *event,
                      gpointer user_data)
{
  ol_assert_ret (GTK_IS_WINDOW (widget), FALSE);
  OlScrollModule *module = (OlScrollModule*) user_data;
  if (module == NULL)
    return FALSE;
  gint width, height;
  OlConfig *config = ol_config_get_instance ();
  gtk_window_get_size (GTK_WINDOW (widget), &width, &height);
  ol_config_set_int_no_emit (config, "ScrollMode", "width", width);
  ol_config_set_int_no_emit (config, "ScrollMode", "height", height);
  return FALSE;
}

static void
_config_change_handler (OlConfig *config,
                        gchar *group,
                        gchar *name,
                        gpointer userdata)
{
  ol_debugf ("%s:[%s]%s\n", __FUNCTION__, group, name);
  static const char *GROUP_NAME = "ScrollMode";
  OlScrollModule *module = (OlScrollModule*) userdata;
  if (module == NULL)
    return;
  OlScrollWindow *window = module->scroll;
  /* OlOsdWindow *osd = OL_OSD_WINDOW (userdata); */
  if (window == NULL || !OL_IS_SCROLL_WINDOW (window))
    return;
  if (strcmp (group, GROUP_NAME) != 0)
    return;
  if (strcmp (name, "font-name") == 0)
  {
    gchar *font = ol_config_get_string (config, GROUP_NAME, "font-name");
    ol_assert (font != NULL);
    ol_scroll_window_set_font_name (window, font);
    g_free (font);
  }
  else if (strcmp (name, "width") == 0 ||
           strcmp (name, "height") == 0)
  {
    gint width = ol_config_get_int (config, GROUP_NAME, "width");
    gint height = ol_config_get_int (config, GROUP_NAME, "height");
    gtk_window_resize (GTK_WINDOW (window), width, height);
  }
  else if (strcmp (name, "active-lrc-color") == 0)
  {
    char *color_str = ol_config_get_string (config, GROUP_NAME, name);
    if (color_str != NULL)
    {
      OlColor color = ol_color_from_string (color_str);
      ol_scroll_window_set_active_color (window, color);
      g_free (color_str);
    }
  }
  else if (strcmp (name, "inactive-lrc-color") == 0)
  {
    char *color_str = ol_config_get_string (config, GROUP_NAME, name);
    if (color_str != NULL)
    {
      OlColor color = ol_color_from_string (color_str);
      ol_scroll_window_set_inactive_color (window, color);
      g_free (color_str);
    }
  }
  else if (strcmp (name, "bg-color") == 0)
  {
    char *color_str = ol_config_get_string (config, GROUP_NAME, name);
    if (color_str != NULL)
    {
      OlColor color = ol_color_from_string (color_str);
      ol_scroll_window_set_bg_color (window, color);
      g_free (color_str);
    }
  }
  else if (strcmp (name, "opacity") == 0)
  {
    double opacity = ol_config_get_double (config, group, name);
    ol_scroll_window_set_bg_opacity (window, opacity);
  }
}

static void
ol_scroll_module_init_scroll (OlScrollModule *module)
{
  ol_assert (module != NULL);
  module->scroll = OL_SCROLL_WINDOW (ol_scroll_window_new ());
  g_object_ref_sink(module->scroll);
  if (module->scroll == NULL)
  {
    return;
  }
  OlConfig *config = ol_config_get_instance ();
  _config_change_handler (config, "ScrollMode", "width", module);
  _config_change_handler (config, "ScrollMode", "font-name", module);
  _config_change_handler (config, "ScrollMode", "active-lrc-color", module);
  _config_change_handler (config, "ScrollMode", "inactive-lrc-color", module);
  _config_change_handler (config, "ScrollMode", "bg-color", module);
  _config_change_handler (config, "ScrollMode", "opacity", module);
  g_signal_connect (module->scroll, "configure-event",
                    G_CALLBACK (_window_configure_cb),
                    module);
  g_signal_connect (config, "changed",
                    G_CALLBACK (_config_change_handler),
                    module);
  
  gtk_widget_show(GTK_WIDGET (module->scroll));
}

OlScrollModule*
ol_scroll_module_new (struct OlDisplayModule *module)
{
  OlScrollModule *priv = g_new (OlScrollModule, 1);
  priv->scroll = NULL;
  priv->lrc = NULL;
  ol_music_info_init (&priv->music_info);
  ol_scroll_module_init_scroll (priv);
  return priv;
}

void
ol_scroll_module_free (struct OlDisplayModule *module)
{
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (module == NULL)
    return;
  if (priv->scroll != NULL)
  {
    gtk_widget_destroy (GTK_WIDGET (priv->scroll));
    priv->scroll = NULL;
  }
  ol_music_info_clear (&priv->music_info);
  g_free (priv);
}

void
ol_scroll_module_set_music_info (struct OlDisplayModule *module, OlMusicInfo *music_info)
{
  ol_assert (music_info != NULL); 
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  ol_music_info_copy (&priv->music_info, music_info);
  /*
  if (priv->scroll != NULL)
  {
    ol_scroll_window_set_lyric (priv->scroll, NULL);
    }*/
}

void
ol_scroll_module_set_played_time (struct OlDisplayModule *module, int played_time)
{
  ol_assert (module != NULL);
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (priv->lrc != NULL && priv->scroll != NULL)
  {
    double percentage;
    int lyric_id;
    ol_lrc_get_lyric_by_time (priv->lrc,
                              played_time,
                              priv->duration,
                              NULL,
                              &percentage,
                              &lyric_id);
    const struct OlLrcItem *info = ol_lrc_get_item (priv->lrc,lyric_id);
    if (lyric_id == -1)
    {
      ol_scroll_window_set_lyric (priv->scroll, -1);
      return;
    }
    else
    {
      /*change to the next lyric line*/
      if (lyric_id != ol_scroll_window_get_current_lyric_id (priv->scroll))
      {
	ol_debugf ("lyris:%s\n",ol_lrc_item_get_lyric(info));
        ol_scroll_window_set_lyric (priv->scroll, ol_lrc_item_get_id (info));
      }
      /*set the percentage of the current lyric line*/
      else
        ol_scroll_window_set_current_percentage (priv->scroll, percentage);
    }
    
  }
  else {
    ol_scroll_window_set_lyric (priv->scroll, -1);
  }
}

void
ol_scroll_module_set_lrc (struct OlDisplayModule *module, struct OlLrc *lrc_file)
{
  ol_log_func ();
  ol_assert (module != NULL);
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  priv->lrc = lrc_file;
  if(priv->scroll == NULL)
  {
      priv->scroll = OL_SCROLL_WINDOW (ol_scroll_window_new ());
  }
  if (lrc_file == NULL)
    ol_scroll_window_set_whole_lyrics(priv->scroll, NULL, 0);
  else {
    /*dump the whole lyrics of a song*/
    int count = ol_lrc_item_count (lrc_file);
    GPtrArray *whole_lyrics = g_ptr_array_new_with_free_func (g_free);
    const struct OlLrcItem *info = NULL;
    int i;
    for (i = 0; i < count; i++) {
      info = ol_lrc_get_item (lrc_file, i);
      g_ptr_array_add (whole_lyrics, g_strdup (ol_lrc_item_get_lyric (info)));
    }
    ol_scroll_window_set_whole_lyrics(priv->scroll, whole_lyrics, count);
    g_ptr_array_unref (whole_lyrics);
  }
}

void
ol_scroll_module_set_duration (struct OlDisplayModule *module, int duration)
{
  ol_assert (module != NULL);
  OlScrollModule *priv = ol_display_module_get_data (module);
  ol_assert (priv != NULL);
  if (module == NULL)
    return;
  priv->duration = duration;
}

struct OlDisplayClass*
ol_scroll_module_get_class ()
{
  struct OlDisplayClass *klass = ol_display_class_new ("scroll",
                                                       (OlDisplayInitFunc) ol_scroll_module_new,
                                                       ol_scroll_module_free);
  /* klass->clear_message = ol_scroll_module_clear_message; */
  /* klass->download_fail_message = ol_scroll_module_download_fail_message; */
  /* klass->search_fail_message = ol_scroll_module_search_fail_message; */
  /* klass->search_message = ol_scroll_module_search_message; */
  klass->set_duration = ol_scroll_module_set_duration;
  klass->set_lrc = ol_scroll_module_set_lrc;
  /* klass->set_message = ol_scroll_module_set_message; */
  klass->set_music_info = ol_scroll_module_set_music_info;
  klass->set_played_time = ol_scroll_module_set_played_time;
  /* klass->set_player = ol_scroll_module_set_player; */
  /* klass->set_status = ol_scroll_module_set_status; */
  return klass;
}
