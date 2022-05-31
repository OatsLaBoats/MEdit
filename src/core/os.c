#include <Windows.h>
#include <assert.h>

#include "os.h"
#include "utils.h"

void set_clipboard(const Char8 *text, int len)
{
    assert(text != NULL);

    if(OpenClipboard(NULL))
    {
        int wsize = MultiByteToWideChar(CP_UTF8, 0, (char *)text, len, NULL, 0);
        if(wsize > 0)
        {
            HGLOBAL mem = (HGLOBAL)GlobalAlloc(GMEM_MOVEABLE, (wsize + 1) * sizeof(wchar_t));
            if(mem != NULL)
            {
                wchar_t* wstr = (wchar_t*)GlobalLock(mem);
                if(wstr != NULL)
                {
                    if(MultiByteToWideChar(CP_UTF8, 0, (char *)text, len, wstr, wsize) > 0)
                    {
                        wstr[wsize] = 0;
                        GlobalUnlock(mem);
                        log_if_win(SetClipboardData(CF_UNICODETEXT, mem) == NULL, "Failed to set clipboard data");
                    }
                    else
                    {
                        log_win_error("Failed to translate utf-8 text to utf-16");
                        GlobalUnlock(mem);
                    }
                }
                else
                {
                    log_win_error("Failed to lock memory");
                }
            }
            else
            {
                log_win_error("Failed to allocate memory");
            }
        }

        log_if_win(!CloseClipboard(), "Failed to close clipboard");
    }
    else
    {
        log_win_error("Failed to open clipboard");
    }
}

Char8 *get_clipboard(int *result_size)
{
    if(!IsClipboardFormatAvailable(CF_UNICODETEXT))
    {
        log_warning("Clipboard content is not supported");
        return NULL;
    }

    if(OpenClipboard(NULL))
    {
        HGLOBAL mem = GetClipboardData(CF_UNICODETEXT); 
        if(mem != NULL)
        {
            SIZE_T size = GlobalSize(mem) - 1;
            if(size > 0)
            {
                LPCWSTR wstr = (LPCWSTR)GlobalLock(mem);
                if(wstr != NULL) 
                {
                    int utf8size = WideCharToMultiByte(CP_UTF8, 0, wstr, (int)(size / sizeof(wchar_t)), NULL, 0, NULL, NULL);
                    if(utf8size > 0)
                    {
                        char* utf8 = (char*)malloc(utf8size);
                        if(utf8 != NULL)
                        {
                            if(WideCharToMultiByte(CP_UTF8, 0, wstr, (int)(size / sizeof(wchar_t)), utf8, utf8size, NULL, NULL) > 0)
                            {
                                *result_size = utf8size;
                                return (Char8 *)utf8;
                            }
                            else
                            {
                                log_win_error("Failed to translate utf-16 text to utf-8");
                                free(utf8);
                            }
                        }
                        else
                        {
                            log_error("Failed to allocate memory for utf-8 text");
                        }
                    }

                    GlobalUnlock(mem);
                }
                else
                {
                    log_win_error("Failed to allocate memory");
                }
            }
        }
        else
        {
            log_win_error("Failed to get clipboard data");
        }

        log_if_win(!CloseClipboard(), "Failed to close clipboard");
    }
    else
    {
        log_win_error("Failed to open clipboard");
    }

    return NULL;
}
