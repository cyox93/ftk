#include "ftk_file_browser.h"
#include "ftk_app_movie.h"

typedef struct _PrivInfo
{
	FtkBitmap* icon;
}PrivInfo;

static FtkBitmap* ftk_app_movie_get_icon(FtkApp* thiz)
{
	DECL_PRIV(thiz, priv);
	const char* name="movie.png";
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

static const char* ftk_app_movie_get_name(FtkApp* thiz)
{
	return _("영화");
}

static Ret ftk_app_movie_run(FtkApp* thiz, int argc, char* argv[])
{
	FtkWidget* win = ftk_file_browser_create(FTK_FILE_BROWER_APP);
	ftk_window_set_animation_hint(win, "app_main_window");
	ftk_file_browser_set_path(win, "/media/DATA/Movies");
	ftk_file_browser_set_filter(win, "video/mpeg");
	ftk_file_browser_load(win);
#ifdef HAS_MAIN
	FTK_QUIT_WHEN_WIDGET_CLOSE(win);
#endif	
	return RET_OK;
}

static void ftk_app_movie_destroy(FtkApp* thiz)
{
	if(thiz != NULL)
	{
		DECL_PRIV(thiz, priv);
		ftk_bitmap_unref(priv->icon);
		FTK_FREE(thiz);
	}

	return;
}

FtkApp* ftk_app_movie_create(void)
{
	FtkApp* thiz = FTK_ZALLOC(sizeof(FtkApp) + sizeof(PrivInfo));

	if(thiz != NULL)
	{
		thiz->run  = ftk_app_movie_run;
		thiz->get_icon = ftk_app_movie_get_icon;
		thiz->get_name = ftk_app_movie_get_name;
		thiz->destroy = ftk_app_movie_destroy;
	}

	return thiz;
}
