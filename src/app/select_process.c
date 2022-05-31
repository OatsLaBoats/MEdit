#include "../core/utils.h"

#include "select_process.h"

static void scan_active_processes(App *app)
{
    ProcessArray *processes = app->sp.processes;

    get_process_array(processes);
    app->sp.combo_len = 0;
    for(int i = 0; i < processes->len; ++i)
    {
        if(processes->pids[i] == 0)
        {
            continue;
        }

        char process_name[PROCESS_NAME_LEN];
        Char8 *buffer = app->sp.combo_state[app->sp.combo_len];
        if(!get_process_name(process_name, PROCESS_NAME_LEN, processes->pids[i]))
        {
            size_t process_memsize = 0;
            get_process_memory_usage(processes->pids[i], &process_memsize);

            sprintf_s((char *)buffer, PROCESS_NAME_LEN, "%s   %.1f MB", process_name, (double)process_memsize / (double)megabytes(1));

            app->sp.valid_pids[app->sp.combo_len] = processes->pids[i];
            ++app->sp.combo_len;
        }
    }
}

int sp_init(App *app)
{
    app->sp.processes = calloc(1, sizeof(ProcessArray));
    return_if(app->sp.processes == NULL, "Failed to allocate process array", -1);

    int combo_max_choices = 2048;
    app->sp.combo_state = malloc(sizeof(char *) * combo_max_choices);
    return_if(app->sp.combo_state == NULL, "Failed to allocate combo state", -1);

    Char8 *s_buffer = malloc(PROCESS_NAME_LEN * combo_max_choices);
    return_if(s_buffer == NULL, "Failed to allocate combo string buffer", -1);

    for(int i = 0; i < combo_max_choices; ++i)
    {
        app->sp.combo_state[i] = s_buffer + i * PROCESS_NAME_LEN;
    }

    app->sp.valid_pids = malloc(combo_max_choices * sizeof(DWORD));
    return_if(app->sp.valid_pids == NULL, "Failed to allocate combo valid pid buffer", -1);

    scan_active_processes(app);

    return 0;
}

static void sp_scan_button(App *app, struct nk_context *ctx)
{
    if(nk_button_label(ctx, "scan"))
    {
        scan_active_processes(app);
        log_message("Scanned %d processes", app->sp.combo_len);
    }
}

static void sp_select_button(App *app, struct nk_context *ctx)
{
    if(nk_button_label(ctx, "select"))
    {
        process_destroy(app->shared.process);

        DWORD pid = app->sp.valid_pids[app->sp.combo_result];

        if(!process_init(app->shared.process, pid))
        {
            log_message("Selected process %s %lu", app->sp.combo_state[app->sp.combo_result], pid);
        }
    }
}

static void sp_process_list(App *app, struct nk_context *ctx)
{
    const char **combo_state = (const char **)app->sp.combo_state;
    app->sp.combo_result = nk_combo(ctx, combo_state, app->sp.combo_len, app->sp.combo_result, 25, nk_vec2(200, 200));
}

void sp_update(App *app, struct nk_context *ctx)
{
    if(nk_begin(ctx, "Select Process", nk_rect(0, 1, 200, 110), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(ctx, 40, 2);
            sp_scan_button(app, ctx);
            sp_select_button(app, ctx);
        
        nk_layout_row_dynamic(ctx, 25, 1);
            sp_process_list(app, ctx);
    } 
    nk_end(ctx);
}

