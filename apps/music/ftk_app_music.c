#include "ftk_file_browser.h"
#include "ftk_app_music.h"

#define IDC_PLAY		100
#define IDC_STOP		101
#define IDC_TITLE		102

typedef struct _PrivInfo
{
	FtkBitmap* icon;
}PrivInfo;

static FtkBitmap* ftk_app_music_get_icon(FtkApp* thiz)
{
	DECL_PRIV(thiz, priv);
	const char* name="music.png";
	char file_name[FTK_MAX_PATH + 1] = {0};
	return_val_if_fail(priv != NULL, NULL);
	
	if(priv->icon != NULL) return priv->icon;
	
	snprintf(file_name, FTK_MAX_PATH, "%s/icons/%s", APP_DATA_DIR, name);
	priv->icon = ftk_bitmap_factory_load(ftk_default_bitmap_factory(), file_name);
	if(priv->icon != NULL) return priv->icon;

	snprintf(file_name, FTK_MAX_PATH, "%s/icons/%s", APP_LOCAL_DATA_DIR, name);
	priv->icon = ftk_bitmap_factory_load(ftk_default_bitmap_factory(), file_name);

	return priv->icon;
}

static const char* ftk_app_music_get_name(FtkApp* thiz)
{
	return _("음악");
}

static Ret _app_music_selected(void* ctx, int index, const char* path)
{
	char temp[200];
	FtkWidget *win = (FtkWidget *)ctx;
	FtkWidget *label;

	label = ftk_widget_lookup(win, IDC_TITLE);

	if (path) {
		if (label) {
			char *title = strrchr(path, '/');
			if (!title) title = path;
			else title++;

			ftk_widget_set_text(label, title);
		}

		sprintf(temp, "/usr/bin/madplay %c%s%c &", '"', path, '"');
		system(temp);
	}

	return RET_OK;
}

static Ret _app_music_browser(FtkWidget *thiz)
{
	FtkWidget* win = ftk_file_browser_create(FTK_FILE_BROWER_SINGLE_CHOOSER);
	ftk_window_set_animation_hint(win, "app_main_window");
	ftk_file_browser_set_path(win, "/media/DATA/Musics");
	ftk_file_browser_set_filter(win, "audio/mp3");
	ftk_file_browser_set_choosed_handler(win, _app_music_selected, thiz);
	ftk_file_browser_load(win);

	return RET_OK;
}

const char* ftk_translate_path(const char* path, char out_path[FTK_MAX_PATH+1])
{
	snprintf(out_path, FTK_MAX_PATH, "%s/%s", APP_DATA_DIR, path);
	if(access(out_path, R_OK) < 0)
	{
		snprintf(out_path, FTK_MAX_PATH, "%s/%s", APP_LOCAL_DATA_DIR, path);
	}
	ftk_logd("%s->%s\n", path, out_path);

	return out_path;
}

static Ret ftk_music_on_button_clicked(void* ctx, void* obj)
{
	FtkWidget* button = (FtkWidget *)obj;
	FtkWidget* win = (FtkWidget *)ctx;
	FtkWidget* label = (FtkWidget *)ctx;

	return_val_if_fail(obj != NULL && win != NULL, RET_FAIL);

	system("killall madplay");

	switch (ftk_widget_id(button)) {
	case IDC_PLAY:
		_app_music_browser(win);
		break;
	case IDC_STOP:
		label = ftk_widget_lookup(win, IDC_TITLE);
		if (label)
			ftk_widget_set_text(label, "");
		break;
	default:
		break;
	}

	return RET_OK;
}

static void _app_music_create_button(FtkWidget *win, int x, int id, const char *icons)
{
	FtkGc gc = {0};
	char temp[100];
	char path[FTK_MAX_PATH+1] = {0};

	FtkWidget* button = NULL;
	FtkBitmap* bitmap_normal = NULL;
	FtkBitmap* bitmap_active = NULL;
	FtkBitmap* bitmap_focus = NULL;

	gc.mask = FTK_GC_BITMAP;

	sprintf(temp, "icons/%s_normal.png", icons);
	bitmap_normal =  ftk_bitmap_factory_load(ftk_default_bitmap_factory(),
			ftk_translate_path(temp, path));

	sprintf(temp, "icons/%s_pressed.png", icons);
	bitmap_active =  ftk_bitmap_factory_load(ftk_default_bitmap_factory(),
			ftk_translate_path(temp, path));

	sprintf(temp, "icons/%s_selected.png", icons);
	bitmap_focus =  ftk_bitmap_factory_load(ftk_default_bitmap_factory(),
			ftk_translate_path(temp, path));

	button = ftk_button_create(win, x, 170, 68, 68);
	ftk_widget_set_id(button, id);
	ftk_button_set_clicked_listener(button, ftk_music_on_button_clicked, win);

	gc.bitmap = bitmap_normal;
	ftk_widget_set_gc(button, FTK_WIDGET_NORMAL, &gc);
	gc.bitmap = bitmap_focus;
	ftk_widget_set_gc(button, FTK_WIDGET_FOCUSED, &gc);
	gc.bitmap = bitmap_active;
	ftk_widget_set_gc(button, FTK_WIDGET_ACTIVE, &gc);

	ftk_bitmap_unref(bitmap_normal);
	ftk_bitmap_unref(bitmap_active);
	ftk_bitmap_unref(bitmap_focus);

}

static Ret ftk_music_on_shutdown(void* ctx, void* obj)
{
	system("killall madplay");
	ftk_widget_unref(ctx);

	return RET_OK;
}

static Ret ftk_clock_on_prepare_options_menu(void* ctx, FtkWidget* menu_panel)
{
	FtkWidget* item;
	
	item = ftk_menu_item_create(menu_panel);
	ftk_widget_set_text(item, _("Quit"));
	ftk_menu_item_set_clicked_listener(item, ftk_music_on_shutdown, ctx);
	ftk_widget_show(item, 1);

	return RET_OK;
}

static Ret ftk_app_music_run(FtkApp* thiz, int argc, char* argv[])
{
	FtkWidget *label;

	FtkWidget *win = ftk_app_window_create();
	ftk_window_set_animation_hint(win, "app_main_window");

	label = ftk_label_create(win, 10, 50, 220, 100);
	ftk_widget_set_id(label, IDC_TITLE);

	_app_music_create_button(win, 35, IDC_PLAY, "play");
	_app_music_create_button(win, 137, IDC_STOP, "stop");

	ftk_app_window_set_on_prepare_options_menu(win, ftk_clock_on_prepare_options_menu, win);
	ftk_widget_show_all(win, 1);

#ifdef HAS_MAIN
	FTK_QUIT_WHEN_WIDGET_CLOSE(win);
#endif	
	return RET_OK;
}

static void ftk_app_music_destroy(FtkApp* thiz)
{
	if(thiz != NULL)
	{
		DECL_PRIV(thiz, priv);
		ftk_bitmap_unref(priv->icon);
		FTK_FREE(thiz);
	}

	return;
}

FtkApp* ftk_app_music_create(void)
{
	FtkApp* thiz = FTK_ZALLOC(sizeof(FtkApp) + sizeof(PrivInfo));

	if(thiz != NULL)
	{
		thiz->run  = ftk_app_music_run;
		thiz->get_icon = ftk_app_music_get_icon;
		thiz->get_name = ftk_app_music_get_name;
		thiz->destroy = ftk_app_music_destroy;
	}

	return thiz;
}
