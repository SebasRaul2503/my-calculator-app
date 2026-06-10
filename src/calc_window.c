#include "calc_window.h"
#include "evaluator.h"
#include "memory_reg.h"
#include "history.h"
#include <stdio.h>
#include <string.h>

#define CALC_FMT "%.10g"

typedef enum {
    BTN_INSERT, BTN_EQUALS, BTN_CLEAR, BTN_BACK,
    BTN_MC, BTN_MR, BTN_MPLUS, BTN_MMINUS
} BtnKind;

typedef struct { const char *label; const char *insert; BtnKind kind; int row; int col; int colspan; } Btn;

static const Btn BUTTONS[] = {
    {"sin","sin(",BTN_INSERT,0,0,1}, {"cos","cos(",BTN_INSERT,0,1,1}, {"tan","tan(",BTN_INSERT,0,2,1}, {"^","^",BTN_INSERT,0,3,1}, {"sqrt","sqrt(",BTN_INSERT,0,4,1},
    {"asin","asin(",BTN_INSERT,1,0,1}, {"acos","acos(",BTN_INSERT,1,1,1}, {"atan","atan(",BTN_INSERT,1,2,1}, {"ln","ln(",BTN_INSERT,1,3,1}, {"log","log(",BTN_INSERT,1,4,1},
    {"pi","pi",BTN_INSERT,2,0,1}, {"e","e",BTN_INSERT,2,1,1}, {"exp","exp(",BTN_INSERT,2,2,1}, {"abs","abs(",BTN_INSERT,2,3,1}, {"%","%",BTN_INSERT,2,4,1},
    {"(","(",BTN_INSERT,3,0,1}, {")",")",BTN_INSERT,3,1,1}, {"MC",NULL,BTN_MC,3,2,1}, {"MR",NULL,BTN_MR,3,3,1}, {"M+",NULL,BTN_MPLUS,3,4,1},
    {"7","7",BTN_INSERT,4,0,1}, {"8","8",BTN_INSERT,4,1,1}, {"9","9",BTN_INSERT,4,2,1}, {"/","/",BTN_INSERT,4,3,1}, {"M-",NULL,BTN_MMINUS,4,4,1},
    {"4","4",BTN_INSERT,5,0,1}, {"5","5",BTN_INSERT,5,1,1}, {"6","6",BTN_INSERT,5,2,1}, {"*","*",BTN_INSERT,5,3,1}, {"C",NULL,BTN_CLEAR,5,4,1},
    {"1","1",BTN_INSERT,6,0,1}, {"2","2",BTN_INSERT,6,1,1}, {"3","3",BTN_INSERT,6,2,1}, {"-","-",BTN_INSERT,6,3,1}, {"\xe2\x8c\xab",NULL,BTN_BACK,6,4,1},
    {"0","0",BTN_INSERT,7,0,1}, {".",".",BTN_INSERT,7,1,1}, {"+","+",BTN_INSERT,7,2,1}, {"=",NULL,BTN_EQUALS,7,3,2},
};

typedef struct {
    GtkWidget *window;
    GtkWidget *entry;
    GtkWidget *result_label;
    GtkWidget *mem_label;
    GtkWidget *deg_toggle;
    GtkWidget *history_list;
    GtkWidget *history_revealer;
    MemoryReg mem;
    History hist;
    double last_result;
    gboolean has_last_result;
} CalcApp;

static void insert_text(CalcApp *c, const char *text) {
    int pos = gtk_editable_get_position(GTK_EDITABLE(c->entry));
    gtk_editable_insert_text(GTK_EDITABLE(c->entry), text, -1, &pos);
    gtk_editable_set_position(GTK_EDITABLE(c->entry), pos);
    gtk_widget_grab_focus(c->entry);
}

static void update_mem_label(CalcApp *c) {
    gtk_label_set_text(GTK_LABEL(c->mem_label),
                       memreg_has_value(&c->mem) ? "M" : "");
}

static const char *err_message(CalcStatus s) {
    switch (s) {
        case CALC_ERR_DIV_ZERO:     return "Error: división por cero";
        case CALC_ERR_PAREN:        return "Error: paréntesis";
        case CALC_ERR_UNKNOWN_FUNC: return "Error: función desconocida";
        case CALC_ERR_DOMAIN:       return "Error: dominio";
        case CALC_ERR_OVERFLOW:     return "Error: resultado no finito";
        default:                    return "Error de sintaxis";
    }
}

static void history_add_row(CalcApp *c, const char *expr, double result) {
    char text[320];
    snprintf(text, sizeof(text), "%s = " CALC_FMT, expr, result);
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *lbl = gtk_label_new(text);
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), lbl);
    g_object_set_data_full(G_OBJECT(row), "expr", g_strdup(expr), g_free);
    gtk_list_box_insert(GTK_LIST_BOX(c->history_list), row, 0);
}

static void do_evaluate(CalcApp *c) {
    const char *expr = gtk_editable_get_text(GTK_EDITABLE(c->entry));
    if (expr == NULL || expr[0] == '\0') return;
    CalcAngleMode mode =
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->deg_toggle))
            ? CALC_MODE_RAD : CALC_MODE_DEG;
    double result;
    CalcStatus s = calc_eval(expr, mode, &result);
    if (s != CALC_OK) {
        gtk_label_set_text(GTK_LABEL(c->result_label), err_message(s));
        return;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), CALC_FMT, result);
    gtk_label_set_text(GTK_LABEL(c->result_label), buf);
    c->last_result = result;
    c->has_last_result = TRUE;
    history_add(&c->hist, expr, result);
    history_add_row(c, expr, result);
}

static void on_button_clicked(GtkButton *btn, gpointer user_data) {
    CalcApp *c = user_data;
    BtnKind kind = (BtnKind)GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "kind"));
    const char *insert = g_object_get_data(G_OBJECT(btn), "insert");
    switch (kind) {
        case BTN_INSERT: insert_text(c, insert); break;
        case BTN_EQUALS: do_evaluate(c); break;
        case BTN_CLEAR:
            gtk_editable_set_text(GTK_EDITABLE(c->entry), "");
            gtk_label_set_text(GTK_LABEL(c->result_label), "");
            break;
        case BTN_BACK: {
            int pos = gtk_editable_get_position(GTK_EDITABLE(c->entry));
            if (pos > 0)
                gtk_editable_delete_text(GTK_EDITABLE(c->entry), pos - 1, pos);
            break;
        }
        case BTN_MC: memreg_clear(&c->mem); update_mem_label(c); break;
        case BTN_MR:
            if (memreg_has_value(&c->mem)) {
                char buf[64];
                snprintf(buf, sizeof(buf), CALC_FMT, memreg_recall(&c->mem));
                insert_text(c, buf);
            }
            break;
        case BTN_MPLUS:
            if (c->has_last_result) { memreg_add(&c->mem, c->last_result); update_mem_label(c); }
            break;
        case BTN_MMINUS:
            if (c->has_last_result) { memreg_sub(&c->mem, c->last_result); update_mem_label(c); }
            break;
    }
}

static void on_entry_activate(GtkEntry *entry, gpointer user_data) {
    (void)entry;
    do_evaluate(user_data);
}

static void on_deg_toggled(GtkToggleButton *t, gpointer user_data) {
    (void)user_data;
    gtk_button_set_label(GTK_BUTTON(t),
        gtk_toggle_button_get_active(t) ? "RAD" : "DEG");
}

static void on_hist_toggled(GtkToggleButton *t, gpointer user_data) {
    CalcApp *c = user_data;
    gtk_revealer_set_reveal_child(GTK_REVEALER(c->history_revealer),
                                  gtk_toggle_button_get_active(t));
}

static void on_history_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    (void)box;
    CalcApp *c = user_data;
    const char *expr = g_object_get_data(G_OBJECT(row), "expr");
    if (expr) {
        gtk_editable_set_text(GTK_EDITABLE(c->entry), expr);
        gtk_editable_set_position(GTK_EDITABLE(c->entry), -1);
        gtk_widget_grab_focus(c->entry);
    }
}

GtkWidget *calc_window_new(GtkApplication *app) {
    CalcApp *c = g_new0(CalcApp, 1);
    memreg_init(&c->mem);
    history_init(&c->hist);
    c->has_last_result = FALSE;

    c->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(c->window), "Calc");
    gtk_window_set_default_size(GTK_WINDOW(c->window), 440, 600);
    g_object_set_data_full(G_OBJECT(c->window), "calcapp", c, g_free);

    GtkWidget *outer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_top(outer, 8);
    gtk_widget_set_margin_bottom(outer, 8);
    gtk_widget_set_margin_start(outer, 8);
    gtk_widget_set_margin_end(outer, 8);
    gtk_window_set_child(GTK_WINDOW(c->window), outer);

    GtkWidget *col = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_hexpand(col, TRUE);
    gtk_box_append(GTK_BOX(outer), col);

    GtkWidget *topbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    c->entry = gtk_entry_new();
    gtk_widget_set_hexpand(c->entry, TRUE);
    g_signal_connect(c->entry, "activate", G_CALLBACK(on_entry_activate), c);
    gtk_box_append(GTK_BOX(topbar), c->entry);
    GtkWidget *hist_toggle = gtk_toggle_button_new_with_label("\xe2\x98\xb0");
    g_signal_connect(hist_toggle, "toggled", G_CALLBACK(on_hist_toggled), c);
    gtk_box_append(GTK_BOX(topbar), hist_toggle);
    gtk_box_append(GTK_BOX(col), topbar);

    GtkWidget *resbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    c->mem_label = gtk_label_new("");
    c->result_label = gtk_label_new("");
    gtk_widget_set_halign(c->result_label, GTK_ALIGN_END);
    gtk_widget_set_hexpand(c->result_label, TRUE);
    gtk_box_append(GTK_BOX(resbar), c->mem_label);
    gtk_box_append(GTK_BOX(resbar), c->result_label);
    gtk_box_append(GTK_BOX(col), resbar);

    c->deg_toggle = gtk_toggle_button_new_with_label("DEG");
    gtk_widget_set_halign(c->deg_toggle, GTK_ALIGN_START);
    g_signal_connect(c->deg_toggle, "toggled", G_CALLBACK(on_deg_toggled), NULL);
    gtk_box_append(GTK_BOX(col), c->deg_toggle);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 4);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 4);
    gtk_widget_set_vexpand(grid, TRUE);
    for (gsize i = 0; i < G_N_ELEMENTS(BUTTONS); i++) {
        const Btn *b = &BUTTONS[i];
        GtkWidget *btn = gtk_button_new_with_label(b->label);
        gtk_widget_set_hexpand(btn, TRUE);
        gtk_widget_set_vexpand(btn, TRUE);
        g_object_set_data(G_OBJECT(btn), "kind", GINT_TO_POINTER(b->kind));
        g_object_set_data(G_OBJECT(btn), "insert", (gpointer)b->insert);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_button_clicked), c);
        gtk_grid_attach(GTK_GRID(grid), btn, b->col, b->row, b->colspan, 1);
    }
    gtk_box_append(GTK_BOX(col), grid);

    c->history_revealer = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(c->history_revealer),
                                     GTK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT);
    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scrolled, 200, -1);
    c->history_list = gtk_list_box_new();
    g_signal_connect(c->history_list, "row-activated",
                     G_CALLBACK(on_history_row_activated), c);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), c->history_list);
    gtk_revealer_set_child(GTK_REVEALER(c->history_revealer), scrolled);
    gtk_box_append(GTK_BOX(outer), c->history_revealer);

    return c->window;
}
