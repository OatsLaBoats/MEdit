#include <assert.h>
#include <errno.h>

#include "../core/utils.h"

#include "app.h"
#include "select_process.h"
#include "scanner.h"
#include "editor.h"

int app_init(App *app)
{
    // Initialize shared state
    app->shared.process = calloc(1, sizeof(Process));
    return_if(app->shared.process == NULL, "Failed to allocate process struct", -1);
    return_if(array_init(&app->shared.scanned_addresses, MemoryAddress), "Failed to initialize array for scan results", -1);

    // Initialize state for process selector
    return_if_s(sp_init(app), -1);

    // Initialize state for scanner
    return_if_s(scn_init(app), -1);

    // Initialize state for editor
    return_if_s(edt_init(app), -1);

    return 0;
}

bool app_update(App *app, struct nk_context *ctx, double delta)
{
    app->shared.delta = delta;

    sp_update(app, ctx);
    scn_update(app, ctx);
    edt_update(app, ctx);

    return true;
}

void app_destroy(App *app)
{
    (void)app;
}
