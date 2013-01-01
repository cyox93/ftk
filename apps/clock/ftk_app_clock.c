#include <time.h>
#include "ftk_xul.h"
#include "ftk_expr.h"
#include "ftk_app_clock.h"

typedef enum __AdjustButtonID {
	_CLOCK_ADJUST_BUTTON_HOUR_UP = 100,
	_CLOCK_ADJUST_BUTTON_MINUTE_UP,
	_CLOCK_ADJUST_BUTTON_HOUR_DOWN,
	_CLOCK_ADJUST_BUTTON_MINUTE_DOWN,
} _AdjustButtonID;

typedef enum __ClockImageID {
	_CLOCK_IMAGE_HOUR_10 = 200,
	_CLOCK_IMAGE_HOUR_01,
	_CLOCK_IMAGE_COLON,
	_CLOCK_IMAGE_MINUTE_10,
	_CLOCK_IMAGE_MINUTE_01,
} _ClockImageID;

static FtkBitmap *_digit[10];
static int _hour;
static int _min;

typedef struct _PrivInfo
{
	FtkBitmap* icon;
}PrivInfo;

static Ret ftk_clock_on_button_clicked(void* ctx, void* obj);

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

static Ret _clock_set_time(FtkWidget *thiz)
{
	FtkWidget *image;

	image = ftk_widget_lookup(thiz, _CLOCK_IMAGE_HOUR_10);
	ftk_bitmap_ref(_digit[_hour/10]);
	ftk_image_set_image(image, _digit[_hour/10]);

	image = ftk_widget_lookup(thiz, _CLOCK_IMAGE_HOUR_01);
	ftk_bitmap_ref(_digit[_hour%10]);
	ftk_image_set_image(image, _digit[_hour%10]);

	image = ftk_widget_lookup(thiz, _CLOCK_IMAGE_MINUTE_10);
	ftk_bitmap_ref(_digit[_min/10]);
	ftk_image_set_image(image, _digit[_min/10]);

	image = ftk_widget_lookup(thiz, _CLOCK_IMAGE_MINUTE_01);
	ftk_bitmap_ref(_digit[_min%10]);
	ftk_image_set_image(image, _digit[_min%10]);

	return RET_OK;
}

static FtkWidget* ftk_clock_create_window(void)
{
	int i = 0;
	FtkGc gc = {0};
	FtkWidget* button = NULL;
	FtkWidget* image = NULL;
	FtkBitmap* bitmap = NULL;
	FtkBitmap* bitmap_normal = NULL;
	FtkBitmap* bitmap_active = NULL;
	FtkBitmap* bitmap_focus = NULL;
	char path[FTK_MAX_PATH+1] = {0};
	char filename[FTK_MAX_PATH] = {0};

	time_t now = time(0);
	struct tm* t = localtime(&now);

	FtkWidget* win =  ftk_app_window_create();
	ftk_window_set_animation_hint(win, "app_main_window");

	gc.mask = FTK_GC_BITMAP;

	bitmap_normal =  ftk_bitmap_factory_load(ftk_default_bitmap_factory(),
			ftk_translate_path("icons/up_normal.png", path));
	bitmap_active =  ftk_bitmap_factory_load(ftk_default_bitmap_factory(),
			ftk_translate_path("icons/up_pressed.png", path));
	bitmap_focus =  ftk_bitmap_factory_load(ftk_default_bitmap_factory(),
			ftk_translate_path("icons/up_selected.png", path));


	for (i = 0; i < 2; i++) {
		button = ftk_button_create(win, 10 + (i * 120), 50, 100, 50);
		ftk_widget_set_id(button, _CLOCK_ADJUST_BUTTON_HOUR_UP + i);
		ftk_button_set_clicked_listener(button, ftk_clock_on_button_clicked, win);

		gc.bitmap = bitmap_normal;
		ftk_widget_set_gc(button, FTK_WIDGET_NORMAL, &gc);
	
		gc.bitmap = bitmap_focus;
		ftk_widget_set_gc(button, FTK_WIDGET_FOCUSED, &gc);
		
		gc.bitmap = bitmap_active;
		ftk_widget_set_gc(button, FTK_WIDGET_ACTIVE, &gc);
	}

	ftk_bitmap_unref(bitmap_normal);
	ftk_bitmap_unref(bitmap_active);
	ftk_bitmap_unref(bitmap_focus);

	bitmap_normal =  ftk_bitmap_factory_load(ftk_default_bitmap_factory(),
			ftk_translate_path("icons/down_normal.png", path));
	bitmap_active =  ftk_bitmap_factory_load(ftk_default_bitmap_factory(),
			ftk_translate_path("icons/down_pressed.png", path));
	bitmap_focus =  ftk_bitmap_factory_load(ftk_default_bitmap_factory(),
			ftk_translate_path("icons/down_selected.png", path));

	for (i = 0; i < 2; i++) {
		button = ftk_button_create(win, 10 + (i * 120), 200, 100, 50);
		ftk_widget_set_id(button, _CLOCK_ADJUST_BUTTON_HOUR_DOWN + i);
		ftk_button_set_clicked_listener(button, ftk_clock_on_button_clicked, win);
		gc.bitmap = bitmap_normal;
		ftk_widget_set_gc(button, FTK_WIDGET_NORMAL, &gc);
	
		gc.bitmap = bitmap_focus;
		ftk_widget_set_gc(button, FTK_WIDGET_FOCUSED, &gc);
		
		gc.bitmap = bitmap_active;
		ftk_widget_set_gc(button, FTK_WIDGET_ACTIVE, &gc);
	}

	ftk_bitmap_unref(bitmap_normal);
	ftk_bitmap_unref(bitmap_active);
	ftk_bitmap_unref(bitmap_focus);

	image = ftk_image_create(win, 10, 110, 50, 80);
	ftk_widget_set_id(image, _CLOCK_IMAGE_HOUR_10);

	image = ftk_image_create(win, 60, 110, 50, 80);
	ftk_widget_set_id(image, _CLOCK_IMAGE_HOUR_01);

	image = ftk_image_create(win, 130, 110, 50, 80);
	ftk_widget_set_id(image, _CLOCK_IMAGE_MINUTE_10);

	image = ftk_image_create(win, 180, 110, 50, 80);
	ftk_widget_set_id(image, _CLOCK_IMAGE_MINUTE_01);

	for (i = 0; i < 10; i++) {
		ftk_snprintf(filename, sizeof(filename)-1, "icons/%d.png", i);
		_digit[i] =  ftk_bitmap_factory_load(ftk_default_bitmap_factory(),
			ftk_translate_path(filename, path));
	}

	image = ftk_image_create(win, 110, 110, 20, 80);
	ftk_widget_set_id(image, _CLOCK_IMAGE_COLON);

	bitmap =  ftk_bitmap_factory_load(ftk_default_bitmap_factory(),
		ftk_translate_path("icons/colon.png", path));
	ftk_image_set_image(image, bitmap);

	_hour = t->tm_hour;
	_min = t->tm_min;

	_clock_set_time(win);

	return win;
}

static Ret ftk_clock_on_set(void* ctx, void* obj)
{
	// need to set time 

	ftk_widget_unref(ctx);

	return RET_OK;
}

static Ret ftk_clock_on_shutdown(void* ctx, void* obj)
{
	ftk_widget_unref(ctx);

	return RET_OK;
}

static Ret ftk_clock_on_prepare_options_menu(void* ctx, FtkWidget* menu_panel)
{
	FtkWidget* item;
	
	item = ftk_menu_item_create(menu_panel);
	ftk_widget_set_text(item, _("Set"));
	ftk_menu_item_set_clicked_listener(item, ftk_clock_on_set, ctx);
	ftk_widget_show(item, 1);

	item = ftk_menu_item_create(menu_panel);
	ftk_widget_set_text(item, _("Cancel"));
	ftk_menu_item_set_clicked_listener(item, ftk_clock_on_shutdown, ctx);
	ftk_widget_show(item, 1);

	return RET_OK;
}

static int _update_time(FtkWidget *thiz, FtkWidget *button)
{
	int id = ftk_widget_id(button);

	switch (id) {
		case _CLOCK_ADJUST_BUTTON_HOUR_UP:
			_hour++;
			if (_hour > 23) _hour = 0;
			break;

		case _CLOCK_ADJUST_BUTTON_HOUR_DOWN:
			_hour--;
			if (_hour < 0) _hour = 23;
			break;

		case _CLOCK_ADJUST_BUTTON_MINUTE_UP:
			_min++;
			if (_min > 59) _min = 0;
			break;

		case _CLOCK_ADJUST_BUTTON_MINUTE_DOWN:
			_min--;
			if (_min < 0) _min = 59;
			break;
		default:
			return RET_FAIL;
	}

	_clock_set_time(thiz);

	return RET_OK;
}

static Ret ftk_clock_on_button_clicked(void* ctx, void* obj)
{
	FtkWidget* button = (FtkWidget *)obj;
	FtkWidget* win = (FtkWidget *)ctx;

	return_val_if_fail(obj != NULL && win != NULL, RET_FAIL);

	_update_time(win, button);
	_clock_set_time(win);

	return RET_OK;
}

static FtkBitmap* ftk_app_clock_get_icon(FtkApp* thiz)
{
	DECL_PRIV(thiz, priv);
	const char* name="clock.png";
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

static const char* ftk_app_clock_get_name(FtkApp* thiz)
{
	return _("시계");
}

static Ret ftk_app_clock_run(FtkApp* thiz, int argc, char* argv[])
{
	DECL_PRIV(thiz, priv);
	FtkWidget* win = NULL;
	return_val_if_fail(priv != NULL, RET_FAIL);

	win = ftk_clock_create_window();
	ftk_app_window_set_on_prepare_options_menu(win, ftk_clock_on_prepare_options_menu, win);
	ftk_widget_show_all(win, 1);

#ifdef HAS_MAIN
	FTK_QUIT_WHEN_WIDGET_CLOSE(win);
#endif	

	return RET_OK;
}

static void ftk_app_clock_destroy(FtkApp* thiz)
{
	int i;

	if(thiz != NULL)
	{
		DECL_PRIV(thiz, priv);

		for (i = 0; i < 10; i++) {
			if (_digit[i])
				ftk_bitmap_unref(_digit[i]);
		}

		ftk_bitmap_unref(priv->icon);
		FTK_FREE(thiz);
	}

	return;
}

FtkApp* ftk_app_clock_create(void)
{
	FtkApp* thiz = FTK_ZALLOC(sizeof(FtkApp) + sizeof(PrivInfo));

	if(thiz != NULL)
	{
		thiz->run  = ftk_app_clock_run;
		thiz->get_icon = ftk_app_clock_get_icon;
		thiz->get_name = ftk_app_clock_get_name;
		thiz->destroy = ftk_app_clock_destroy;
	}

	return thiz;
}

