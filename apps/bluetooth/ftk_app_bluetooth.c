#include "ftk_xul.h"
#include "ftk_expr.h"
#include "ftk_app_bluetooth.h"

#include "bt.h"

//#define __TEST__

typedef struct _PrivInfo
{
	FtkBitmap* icon;

}PrivInfo;

#define IDC_INFO 100
#define IDC_LIST 101

static FtkListModel* _model = NULL;
static FtkSource *_timer = NULL;

static BtHd _bt;
BtDevice _devices[BT_SCAN_MAX];
int _bt_dev_num;
int _bt_connect;

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

static Ret _bt_connect_start(void *ctx)
{
	int ret;
	char temp[100];

	FtkWidget *info = ftk_widget_lookup((FtkWidget *)ctx, IDC_INFO);

	ftk_main_loop_remove_source(ftk_default_main_loop(), _timer);
	_timer = NULL;

	printf("%s : try to connect [%d]\n", __func__, _bt_connect);

	ret = bt_connect(_bt, &_devices[_bt_connect]);
	if (ret < 0) {
		sprintf(temp, "접속 실패");
	} else {
		sprintf(temp, "연결 : %s", _devices[_bt_connect].name);
	}

	if (info) ftk_widget_set_text(info, temp);


	return RET_OK;
}

static Ret _bt_on_item_clicked(void *ctx, void *list)
{
	FtkListItemInfo* info = NULL;
	FtkListModel* model = ftk_list_view_get_model(list);
	int i = ftk_list_view_get_selected(list);

	printf("%s : clicked index %d\n", __func__, i);

	ftk_list_model_get_data(model, i, (void**)&info);
	if(info != NULL && !info->disable)
	{
		info->state = !info->state;
	}

	if (i < _bt_dev_num) {
		FtkWidget *label = ftk_widget_lookup((FtkWidget *)ctx, IDC_INFO);
		if (label) ftk_widget_set_text(label, "접속 중");

		_bt_connect = i;
		_timer = ftk_source_timer_create(300, _bt_connect_start, ctx);
		ftk_main_loop_add_source(ftk_default_main_loop(), _timer);
	}

	return RET_OK;
}

static Ret _bt_scan_start(void *ctx)
{
	FtkWidget *info;
	char temp[100];
	int i;

	info = ftk_widget_lookup((FtkWidget *)ctx, IDC_INFO);

	ftk_main_loop_remove_source(ftk_default_main_loop(), _timer);
	_timer = NULL;

	_bt_dev_num = bt_scan(_bt, _devices, BT_SCAN_MAX);
	if (_bt_dev_num > 0) {
		sprintf(temp, "검색 결과 : %d 개", _bt_dev_num);
		for (i = 0; i < _bt_dev_num; i++) {
			FtkListItemInfo info = {0};

			info.text = _devices[i].name;
			ftk_list_model_add(_model, &info);
		}

	} else if (_bt_dev_num == 0) {
		sprintf(temp, "검색 결과 없음");
	} else {
#ifdef __TEST__
		_bt_dev_num = 9;
		for (i = 0; i < 9; i++) {
			FtkListItemInfo info = {0};

			sprintf(temp, "test %d", i);
			info.text = temp;
			ftk_list_model_add(_model, &info);
		}
#endif
		sprintf(temp, "검색 실패");
	}

	if (info) ftk_widget_set_text(info, temp);

	return RET_OK;
}

static FtkWidget* ftk_bluetooth_create_window(void)
{
	FtkWidget *widget;
	FtkListRender* render = NULL;

	FtkWidget* win =  ftk_app_window_create();
	ftk_window_set_animation_hint(win, "app_main_window");

	widget = ftk_label_create(win, 10, 10, 220, 30);
	ftk_widget_set_id(widget, IDC_INFO);
	ftk_widget_set_text(widget, "검색 중");

	widget = ftk_list_view_create(win, 10, 40, 220, 200);
	ftk_list_view_set_clicked_listener(widget, _bt_on_item_clicked, win);
	ftk_widget_set_id(widget, IDC_LIST);

	_model = ftk_list_model_default_create(10);
	render = ftk_list_render_default_create();

	ftk_list_render_default_set_marquee_attr(render,
			FTK_MARQUEE_AUTO | FTK_MARQUEE_RIGHT2LEFT | FTK_MARQUEE_FOREVER);

	ftk_list_view_init(widget, _model, render, 40);
	ftk_list_model_unref(_model);

	_timer = ftk_source_timer_create(300, _bt_scan_start, win);
	ftk_main_loop_add_source(ftk_default_main_loop(), _timer);

	return win;
}

static Ret ftk_bluetooth_on_scan(void* ctx, void* obj)
{
	FtkWidget *info;

	if (_timer) {
		ftk_main_loop_remove_source(ftk_default_main_loop(), _timer);
		_timer = NULL;
	}

	info = ftk_widget_lookup((FtkWidget *)ctx, IDC_INFO);
	if (info) ftk_widget_set_text(info, "검색 중");

	ftk_list_model_reset(_model);

	_timer = ftk_source_timer_create(300, _bt_scan_start, ctx);
	ftk_main_loop_add_source(ftk_default_main_loop(), _timer);

	return RET_OK;
}

static Ret ftk_bluetooth_on_shutdown(void* ctx, void* obj)
{
	if (_timer) {
		ftk_main_loop_remove_source(ftk_default_main_loop(), _timer);
		_timer = NULL;
	}

	ftk_widget_unref(ctx);

	return RET_OK;
}

static Ret ftk_bluetooth_on_prepare_options_menu(void* ctx, FtkWidget* menu_panel)
{
	FtkWidget* item = ftk_menu_item_create(menu_panel);
	ftk_widget_set_text(item, _("검색"));
	ftk_menu_item_set_clicked_listener(item, ftk_bluetooth_on_scan, ctx);
	ftk_widget_show(item, 1);

	item = ftk_menu_item_create(menu_panel);
	ftk_widget_set_text(item, _("Quit"));
	ftk_menu_item_set_clicked_listener(item, ftk_bluetooth_on_shutdown, ctx);
	ftk_widget_show(item, 1);

	return RET_OK;
}

static FtkBitmap* ftk_app_bluetooth_get_icon(FtkApp* thiz)
{
	DECL_PRIV(thiz, priv);
	const char* name="bluetooth.png";
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

static const char* ftk_app_bluetooth_get_name(FtkApp* thiz)
{
	return _("블루투스");
}

static Ret ftk_app_bluetooth_run(FtkApp* thiz, int argc, char* argv[])
{
	
	DECL_PRIV(thiz, priv);
	FtkWidget* win = NULL;
	return_val_if_fail(priv != NULL, RET_FAIL);

	win = ftk_bluetooth_create_window();
	ftk_app_window_set_on_prepare_options_menu(win, ftk_bluetooth_on_prepare_options_menu, win);
	ftk_widget_show_all(win, 1);

#ifdef HAS_MAIN
	FTK_QUIT_WHEN_WIDGET_CLOSE(win);
#endif	

	return RET_OK;
}

static void ftk_app_bluetooth_destroy(FtkApp* thiz)
{
	if(thiz != NULL)
	{
		DECL_PRIV(thiz, priv);
		ftk_bitmap_unref(priv->icon);
		FTK_FREE(thiz);

		if (_bt) {
			bt_close(_bt);
			_bt = NULL;
		}
	}

	return;
}

FtkApp* ftk_app_bluetooth_create(void)
{
	FtkApp* thiz = FTK_ZALLOC(sizeof(FtkApp) + sizeof(PrivInfo));

	if(thiz != NULL)
	{
		thiz->run  = ftk_app_bluetooth_run;
		thiz->get_icon = ftk_app_bluetooth_get_icon;
		thiz->get_name = ftk_app_bluetooth_get_name;
		thiz->destroy = ftk_app_bluetooth_destroy;

		_bt = bt_open();
	}

	return thiz;
}

