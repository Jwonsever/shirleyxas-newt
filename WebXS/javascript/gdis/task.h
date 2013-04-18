#include <time.h>

/* task modes */
enum {RUNNING, QUEUED, KILLED, COMPLETED, REMOVED};

/************/
/* task pak */
/************/
struct task_pak
{
/* control */
gint pid;
gint ppid;
gint status;
gint parent;
gint child;
gint sister;
gint h_sec;
gint sec;
gint min;
gint hour;
gchar *label;
gchar *message;

/* NEW - real time the task has been running for */
gpointer timer;

gchar *status_file;
FILE *status_fp;
gint status_index;
GString *status_text;
/* JJM DEBUG
#ifdef __WIN32
gpointer start_time;
#else
time_t start_time;
#endif
*/
time_t start_time;

gchar *time;
gdouble pcpu;
gdouble pmem;
gdouble progress;
gpointer locked_model;

/* NEW - non blocking call that returns the worker pid */
gint (*function)(gpointer, ...);

/* main task and arguments (run by the child) */
void (*primary)(gpointer, ...);
gpointer ptr1;
/* cleanup task and arguments (run by the parent) */
void (*cleanup) (gpointer, ...);
gpointer ptr2;
};

/* task control */
gint update_task_info(void);
void task_status_update(struct task_pak *);
//void task_dialog(void);
void gui_task_widget(GtkWidget *);
void gui_task_dialog(void);

gint task_sync(const gchar *);

void task_queue_init(void);
void task_queue_free(void);

void task_free(gpointer);
void task_new(const gchar *,
              gpointer, gpointer,
              gpointer, gpointer,
              gpointer);

// NEW - interruptable
void task_add(const gchar *,
              gpointer, gpointer,
              gpointer, gpointer,
              gpointer);

gint exec_gulp(const gchar *, const gchar *);

gint task_fortran_exe(const gchar *, const gchar *, const gchar *);

void task_kill_selected(void);

const gchar *task_executable_required(const gchar *);
gint task_executable_available(const gchar *);

gint task_is_running_or_queued(const gchar *);

