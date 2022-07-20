#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../core/utils.h"
#include "../core/os.h"
#include "../globals.h"

#include "gui.h"

LRESULT gui_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    Gui *gui = get_window_data(window);
    if(gui == NULL) return DefWindowProcW(window, message, wparam, lparam);

    struct nk_context *ctx = &gui->ctx;

    switch(message)
    {
        case WM_CLOSE:
        {
            gui->is_running = false;
        } break;

        case WM_SIZE:
        {
            // Window not yet initialized.
            if(gui->window->handle == NULL) return DefWindowProcW(window, message, wparam, lparam);

            uint32_t width = LOWORD(lparam);
            uint32_t height = HIWORD(lparam);
            gui->window->width = width;
            gui->window->height = height;

            if(renderer_resize(gui->renderer, width, height)) g_medit_error = true;
        } break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            int down = !((lparam >> 31) & 1);
            int ctrl = GetKeyState(VK_CONTROL) & (1 << 15);

            switch (wparam)
            {
                case VK_SHIFT:
                case VK_LSHIFT:
                case VK_RSHIFT: nk_input_key(ctx, NK_KEY_SHIFT, down); return 1;
                case VK_DELETE: nk_input_key(ctx, NK_KEY_DEL, down); return 1;
                case VK_RETURN: nk_input_key(ctx, NK_KEY_ENTER, down); return 1;
                case VK_TAB: nk_input_key(ctx, NK_KEY_TAB, down); return 1;
                case VK_BACK: nk_input_key(ctx, NK_KEY_BACKSPACE, down); return 1;
                case VK_NEXT: nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down); return 1;
                case VK_PRIOR: nk_input_key(ctx, NK_KEY_SCROLL_UP, down); return 1;

                case VK_HOME:
                {
                    nk_input_key(ctx, NK_KEY_TEXT_START, down);
                    nk_input_key(ctx, NK_KEY_SCROLL_START, down);
                    return 1;
                }

                case VK_END:
                {
                    nk_input_key(ctx, NK_KEY_TEXT_END, down);
                    nk_input_key(ctx, NK_KEY_SCROLL_END, down);
                    return 1;
                }

                case VK_LEFT:
                {
                    if(ctrl)
                    {
                        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
                    }
                    else
                    {
                        nk_input_key(ctx, NK_KEY_LEFT, down);
                    }
                    
                    return 1;
                }

                case VK_RIGHT:
                {
                    if(ctrl)
                    {
                        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
                    }
                    else
                    {
                        nk_input_key(ctx, NK_KEY_RIGHT, down);
                    }

                    return 1;
                }

                case 'C':
                {
                    if(ctrl) 
                    {
                        nk_input_key(ctx, NK_KEY_COPY, down);
                        return 1;
                    }
                } break;

                case 'V':
                {
                    if(ctrl)
                    {
                        nk_input_key(ctx, NK_KEY_PASTE, down);
                        return 1;
                    }
                } break;

                case 'X':
                {
                    if(ctrl) 
                    {
                        nk_input_key(ctx, NK_KEY_CUT, down);
                        return 1;
                    }
                } break;

                case 'Z':
                {
                    if(ctrl) 
                    {
                        nk_input_key(ctx, NK_KEY_TEXT_UNDO, down);
                        return 1;
                    }
                } break;

                case 'R':
                {
                    if(ctrl) 
                    {
                        nk_input_key(ctx, NK_KEY_TEXT_REDO, down);
                        return 1;
                    }
                } break;
            }
        } break;

        case WM_CHAR:
        {
            if(wparam >= 32)
            {
                nk_input_unicode(ctx, (nk_rune)wparam);
                return 1;
            }
        } break;

        case WM_LBUTTONDOWN:
        {
            nk_input_button(ctx, NK_BUTTON_LEFT, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
            SetCapture(window);
            return 1;
        } break;

        case WM_LBUTTONUP:
        {
            nk_input_button(ctx, NK_BUTTON_DOUBLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
            nk_input_button(ctx, NK_BUTTON_LEFT, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
            ReleaseCapture();
            return 1;
        } break;

        case WM_RBUTTONDOWN:
        {
            nk_input_button(ctx, NK_BUTTON_RIGHT, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
            SetCapture(window);
            return 1;
        } break;

        case WM_RBUTTONUP:
        {
            nk_input_button(ctx, NK_BUTTON_RIGHT, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
            ReleaseCapture();
            return 1;
        } break;

        case WM_MBUTTONDOWN:
        {
            nk_input_button(ctx, NK_BUTTON_MIDDLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
            SetCapture(window);
            return 1;
        } break;

        case WM_MBUTTONUP:
        {
            nk_input_button(ctx, NK_BUTTON_MIDDLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 0);
            ReleaseCapture();
            return 1;
        } break;

        case WM_MOUSEWHEEL:
        {
            nk_input_scroll(ctx, nk_vec2(0,(float)(short)HIWORD(wparam) / WHEEL_DELTA));
            return 1;
        } break;

        case WM_MOUSEMOVE:
        {
            nk_input_motion(ctx, (short)LOWORD(lparam), (short)HIWORD(lparam));
            return 1;
        } break;

        case WM_LBUTTONDBLCLK:
        {
            nk_input_button(ctx, NK_BUTTON_DOUBLE, (short)LOWORD(lparam), (short)HIWORD(lparam), 1);
            return 1;
        } break;

        default: return DefWindowProcW(window, message, wparam, lparam);
    }
    
    return 0;
}

static float get_text_width_callback(nk_handle handle, float height, const char *text, int len)
{
    (void)height;
    Font *font = handle.ptr;

    if(font == NULL || text == NULL)
    {
        return 0.0f;
    }

    return (float)font_get_text_width(font, (Char8 *)text, len);
}

static void clipboard_copy_callback(nk_handle usr, const char *text, int len)
{
    (void)usr;
    set_clipboard((Char8 *)text, len);
}

static void clipboard_paste_callback(nk_handle usr, struct nk_text_edit *edit)
{
    (void)usr;
    int size = 0;
    Char8 *contents = get_clipboard(&size);
    fn_exit_if_s(contents == NULL);
    log_if(!nk_textedit_paste(edit, (char *)contents, size), "Failed to paste text");
    free(contents);
}

static int nuklear_state_init(Gui *gui)
{
    struct nk_user_font *font = &gui->nuk_font;
    font->userdata = nk_handle_ptr(gui->font);
    font->height = (float)gui->font->height;
    font->width = get_text_width_callback;

    return_if(!nk_init_default(&gui->ctx, font), "Failed to initialize nuklear", -1);

    gui->ctx.clip.copy = clipboard_copy_callback;
    gui->ctx.clip.paste = clipboard_paste_callback;

    return 0;
}

struct nk_context *gui_init(Gui *gui, Window *window, Renderer *renderer, Font *font)
{
    assert(gui != NULL);
    assert(window != NULL);
    assert(renderer != NULL);
    assert(font != NULL);
    assert(sizeof(struct nk_vec2i) == sizeof(short) * 2);

    gui->window = window;
    gui->renderer = renderer;
    gui->font = font;

    return_if_s(nuklear_state_init(gui), NULL);

    gui->is_running = true;

    return &gui->ctx;
}

void gui_destroy(Gui *gui)
{
    nk_free(&gui->ctx);
    clean_struct(gui);
}

bool gui_process_messages(Gui *gui)
{
    if(!gui->is_running)
    {
        return false;
    }

    MSG message;

    nk_input_begin(&gui->ctx);

    while(PeekMessageW(&message, gui->window->handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    
    nk_input_end(&gui->ctx);
    
    return gui->is_running;
}

static inline COLORREF convert_color(struct nk_color c)
{
    return c.r | (c.g << 8) | (c.b << 16);
}

int gui_render(Gui *gui, struct nk_color color)
{
    int result = 0;
    const struct nk_command *cmd;
    Renderer *renderer = gui->renderer;

    renderer_start_drawing(renderer);
    render_fill(renderer, convert_color(color));

    nk_foreach(cmd, &gui->ctx)
    {
        switch(cmd->type)
        {
            case NK_COMMAND_NOP: break;

            case NK_COMMAND_SCISSOR:
            {
                const struct nk_command_scissor *c = (const struct nk_command_scissor *)cmd;
                if(render_clipping_region(renderer, c->x, c->y, c->w, c->h)) result = -1;
            } break;

            case NK_COMMAND_LINE:
            {
                const struct nk_command_line *c = (const struct nk_command_line *)cmd;
                if(render_line(renderer, c->begin.x, c->begin.y, c->end.x, c->end.y, c->line_thickness, convert_color(c->color))) result = -1;
            } break;

            case NK_COMMAND_RECT:
            {
                const struct nk_command_rect *c = (const struct nk_command_rect *)cmd;
                if(render_rect(renderer, c->x, c->y, c->w, c->h, c->rounding, c->line_thickness, convert_color(c->color))) result = -1;
            } break;

            case NK_COMMAND_RECT_FILLED:
            {
                const struct nk_command_rect_filled *c = (const struct nk_command_rect_filled *)cmd;
                render_filled_rect(renderer, c->x, c->y, c->w, c->h, c->rounding, convert_color(c->color));
            } break;

            case NK_COMMAND_CIRCLE:
            {
                const struct nk_command_circle *c = (const struct nk_command_circle *)cmd;
                if(render_circle(renderer, c->x, c->y, c->w, c->h, c->line_thickness, convert_color(c->color))) result = -1;
            } break;

            case NK_COMMAND_CIRCLE_FILLED:
            {
                const struct nk_command_circle_filled *c = (const struct nk_command_circle_filled *)cmd;
                render_filled_circle(renderer, c->x, c->y, c->w, c->h, convert_color(c->color));
            } break;

            case NK_COMMAND_TRIANGLE:
            {
                const struct nk_command_triangle *c = (const struct nk_command_triangle *)cmd;
                if(render_triangle(renderer, c->a.x, c->a.y, c->b.x, c->b.y, c->c.x, c->c.y, c->line_thickness, convert_color(c->color))) result = -1;
            } break;

            case NK_COMMAND_TRIANGLE_FILLED:
            {
                const struct nk_command_triangle_filled *c = (const struct nk_command_triangle_filled *)cmd;
                render_filled_triangle(renderer, c->a.x, c->a.y, c->b.x, c->b.y, c->c.x, c->c.y, convert_color(c->color));
            } break;

            case NK_COMMAND_POLYGON:
            {
                const struct nk_command_polygon *c = (const struct nk_command_polygon *)cmd;
                if(render_polygon(renderer, (short *)c->points, c->point_count, c->line_thickness, convert_color(c->color))) result = -1;
            } break;

            case NK_COMMAND_POLYGON_FILLED:
            {
                const struct nk_command_polygon_filled *c = (const struct nk_command_polygon_filled *)cmd;
                render_filled_polygon(renderer, (short *)c->points, c->point_count, convert_color(c->color));
            } break;

            case NK_COMMAND_POLYLINE:
            {
                const struct nk_command_polyline *c = (const struct nk_command_polyline *)cmd;
                if(render_polyline(renderer, (short *)c->points, c->point_count, c->line_thickness, convert_color(c->color))) result = -1;
            } break;

            case NK_COMMAND_TEXT:
            {
                const struct nk_command_text *c = (const struct nk_command_text *)cmd;
                if(render_text(renderer, c->font->userdata.ptr, c->x, c->y, c->w, c->h, c->string, c->length, convert_color(c->background), convert_color(c->foreground)))
                    result = -1;
            } break;

            case NK_COMMAND_CURVE:
            {
                const struct nk_command_curve *c = (const struct nk_command_curve *)cmd;

                int16_t p1[2] = {c->begin.x, c->begin.y};
                int16_t p2[2] = {c->ctrl[0].x, c->ctrl[0].y};
                int16_t p3[2] = {c->ctrl[1].x, c->ctrl[1].y};
                int16_t p4[2] = {c->end.x, c->end.y};

                if(render_curve(renderer, p1, p2, p3, p4, c->line_thickness, convert_color(c->color))) result = -1;
            } break;

            case NK_COMMAND_RECT_MULTI_COLOR:
            {
                const struct nk_command_rect_multi_color *c = (const struct nk_command_rect_multi_color *)cmd;
                render_multi_color_rect(renderer, c->x, c->y, c->w, c->h, convert_color(c->left), convert_color(c->top), convert_color(c->right), convert_color(c->bottom));
            } break;

            case NK_COMMAND_IMAGE:
            {
                const struct nk_command_image *c = (const struct nk_command_image *)cmd;
                if(render_image(renderer, c->x, c->y, c->w, c->h, (HBITMAP)c->img.handle.ptr)) result = -1;
            } break;

            case NK_COMMAND_ARC:
            case NK_COMMAND_ARC_FILLED:
            default: break;
        }
    }

    nk_clear(&gui->ctx);
    return_if_s(renderer_end_drawing(renderer), -1);

    return result;
}
