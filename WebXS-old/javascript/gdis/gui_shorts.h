
/* fill types for box packing */
enum {FF, TF, FT, TT, LB};

/* generation 2 shortcuts */
void gui_relation_update(gpointer);
void gui_relation_update_widget(gpointer);

GtkWidget *gui_icon_button(const gchar *, const gchar *,
                             gpointer, gpointer,
                             GtkWidget *);
GtkWidget *gui_stock_button(const gchar *,
                              gpointer, gpointer,
                              GtkWidget *);

GtkWidget *gui_image_button(gint, gpointer, gpointer);

/* TODO - make auto_check use same protoype as direct_check */
GtkWidget *gui_auto_check(gchar *, gpointer , gpointer , gint *, GtkWidget *);
GtkWidget *gui_direct_check(gchar *, gint *, gpointer , gpointer , GtkWidget *);

/* my shortcut routines for gtk interface construction */
GtkWidget *new_check_button(gchar *, gpointer, gpointer, gint, GtkWidget *);

GtkWidget *gui_button(gchar *, gpointer, gpointer, GtkWidget *, gint);
GtkWidget *gui_button_x(gchar *, gpointer, gpointer, GtkWidget *);

void gui_label_button(gchar *, gpointer, gpointer, GtkWidget *);

void new_radio_group(gint, GtkWidget *, gint);

void gui_checkbox_refresh(GtkWidget *, GtkWidget *);

gpointer gui_radio_new(gint *, GtkWidget *);
void gui_radio_button_add(const gchar *, gint, gpointer);

GtkWidget *add_radio_button(gchar *, gpointer, gpointer);

GtkWidget *new_spinner(gchar *, gdouble, gdouble, gdouble,
                       gpointer, gpointer, GtkWidget *);

GtkWidget *gui_auto_spin(gchar *, gdouble *, gdouble, gdouble, gdouble,
                           gpointer, gpointer, GtkWidget *);
GtkWidget *gui_direct_spin(gchar *, gdouble *, gdouble, gdouble, gdouble,
                             gpointer, gpointer, GtkWidget *);

GtkWidget *gui_text_spin_new(const gchar *, gdouble *, gdouble, gdouble, gdouble);

GtkWidget *gui_new_spin(gchar *, gdouble, gdouble, gdouble,
                          gpointer, GtkWidget *);

GtkWidget *gui_direct_hscale(gdouble, gdouble, gdouble,
                               gpointer, gpointer, gpointer, GtkWidget *);

GtkWidget *gui_auto_text_label(gchar **);
GtkWidget *gui_auto_int_label(gint *);
GtkWidget *gui_auto_float_label(gdouble *);

GtkWidget *gui_text_window(gchar **, gint);

GtkWidget *gui_text_entry(gchar *, gchar **, gint, gint, GtkWidget *);

GtkWidget *gui_frame_vbox(const gchar *, gint, gint, GtkWidget *);
GtkWidget *gui_frame_hbox(const gchar *, gint, gint, GtkWidget *);

gpointer gui_pulldown_new(const gchar *, GList *, gint, GtkWidget *);
const gchar *gui_pulldown_text(gpointer);

void gui_colour_box(const gchar *, gdouble *, GtkWidget *);

gpointer gui_pd_new(gchar *, gchar **, GSList *, GtkWidget *);
void gui_pd_list_set(const GSList *, gpointer);
gchar *gui_pd_text(gpointer);
gint gui_pd_index_get(gpointer);
void gui_pd_edit_set(gint, gpointer);

void gui_hbox_pack(GtkWidget *, gchar *, GtkWidget *, gint);

gpointer gui_scrolled_text_new(gint, GtkWidget *);
void gui_scrolled_text_set(gpointer, const gchar *);
gchar *gui_scrolled_text_get(gpointer);

gpointer gui_scrolled_list_new(GtkWidget *);
void gui_scrolled_list_refresh(GSList *, gpointer);
GSList *gui_scrolled_list_selected_get(gpointer);

gpointer gui_file_filter(gpointer, GtkWidget *);
void gui_file_filter_set(gint, gpointer);

GtkWidget *gui_toggled_vbox(const gchar *, gint, GtkWidget *);
GtkWidget *gui_checkbox(gchar *, gint *, GtkWidget *);

GtkWidget *gui_spin_new(gdouble *, gdouble, gdouble, gdouble);
void gui_spin_modify(GtkWidget *, gdouble *);

gpointer gui_notebook_new(GtkWidget *);
void gui_notebook_page_append(gpointer, const gchar *, GtkWidget *);
void gui_notebook_page_visible_set(gpointer, const gchar *, gint);
gint gui_notebook_page_visible_get(gpointer, const gchar *);

void gui_model_pulldown_refresh(gpointer);
gpointer gui_model_pulldown_new(gpointer);
gpointer gui_model_pulldown_widget_get(gpointer);
gpointer gui_model_pulldown_active(gpointer);

