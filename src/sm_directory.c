/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
Directory [mock] class
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "sm_directory.h"

// Private methods

static sm_directory_record *find_record(sm_directory *t, const char *name)
{
    sm_directory_record *r = t->top;
    while (r != NULL && strcmp(r->name, name))
    {
        r = r->next;
    }
    return r;
}

static char *find_name(sm_directory *t, void *ptr)
{
    sm_directory_record *r = t->top;
    while (r != NULL && ptr != r->ptr)
    {
        r = r->next;
    }
    return r->name;
}

// Public methods

sm_directory *sm_directory_create()
{
    sm_directory *t;
    if ((t = malloc(sizeof(sm_directory))) == NULL)
    {
        SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
        return NULL;
    }
    t->top = NULL;
    return t;
}

sm_directory *sm_directory_set(sm_directory *t, const char *name, void *ptr)
{
    sm_directory_record *r = find_record(t, name);
    if (t->top == NULL || r == NULL)
    {
        if ((r = malloc(sizeof(sm_directory_record))) == NULL)
        {
            SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
            return NULL;
        }
        if ((r->name = malloc(strlen(name))) == NULL)
        {
            SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
            free(r);
            return NULL;
        }
        strcpy(r->name, name);
        r->ref = &(r->ptr);
        r->ptr = ptr;
        r->prev = NULL;
        r->next = t->top;
        if (t->top != NULL)
            r->next->prev = r;
        t->top = r;
    }
    else
    {
        char *r_n = r->name;
        if ((r->name = malloc(strlen(name))) == NULL)
        {
            SM_REPORT_MESSAGE(SM_LOG_ERR, "malloc() failed");
            r->name = r_n;
            return NULL;
        }
        free(r_n);
        strcpy(r->name, name);
        r->ptr = ptr;
    }
    return t;
}

void **sm_directory_get_ref(sm_directory *t, const char *name)
{
    sm_directory_record *tr = find_record(t, name);
    if (tr == NULL)
        return NULL;
    else
        return tr->ref;
}

char *sm_directory_get_name(sm_directory *t, void *ptr)
{
    return find_name(t, ptr);
}

void sm_directory_remove(sm_directory *t, const char *name)
{
    sm_directory_record *tr = find_record(t, name);
    if (tr != NULL)
    {
        tr->prev->next = tr->next;
        tr->next->prev = tr->prev;
        free(tr->name);
        free(tr);
    }
}

void sm_directory_free(sm_directory *t)
{
    sm_directory_record *tmp;
    while (t->top != NULL)
    {
        tmp = t->top->next;
        free(t->top);
        t->top->next = tmp;
    }
}

int sm_directory_to_string(sm_directory *d, char *buffer)
{
    char *s = buffer;
    if (SM_UNLIKELY(d == NULL))
    {
        s += sprintf(s, "NULL\n");
    }
    else
    {
        s += sprintf(s, "address: %p\n", d);
        sm_directory_record *rec = d->top;
        size_t num = 0;
        while (rec != NULL)
        {
            rec = rec->next;
            ++num;
        }
        s += sprintf(s, "directory size: %lu\n", num);
    }
    return (int)((char *)s - (char *)buffer);
}
