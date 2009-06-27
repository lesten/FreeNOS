/*
 * Copyright (C) 2009 Niek Linnenbank
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <API/VMCtl.h>
#include <FreeNOS/Memory.h>
#include <Macros.h>
#include <Config.h>
#include <Types.h>
#include <ListAllocator.h>
#include <LogMessage.h>
#include "MemoryServer.h"
#include <stdlib.h>

int main(int argc, char **argv)
{
    MemoryServer server;
    return server.run();
}

extern C int __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle)
{
    return (0);
}

extern C void __cxa_pure_virtual()
{
}

extern C void __dso_handle()
{
}

extern C void __stack_chk_fail(void)
{
}

void __assertFailure(const char *fmt, ...)
{
    va_list args;
    char msg[256];

    /* Construct assertion failure message. */
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    
    /* Send it to the log server. */
    log("%s", msg);

    /* Terminate immediately. */
    exit(EXIT_FAILURE);
}

int __assertRead(Address addr)
{
    return VMCtl(Access, SELF, ZERO, addr, PAGE_PRESENT|PAGE_USER);
}

int __assertWrite(Address addr)
{
    return VMCtl(Access, SELF, ZERO, addr, PAGE_PRESENT|PAGE_USER|PAGE_RW);
}

extern C void SECTION(".entry") _entry() 
{
    char *argv[] = {"main", ZERO };

    /* Allocate our own heap. */
    for (Size i = 0; i < 4; i++)
    {
	VMCtl(Map, SELF, ZERO, HEAP_START + (PAGESIZE * i));
    }
    /* Create allocator instance. */
    ListAllocator *li = new (HEAP_START) ListAllocator();
    li->region(HEAP_START + sizeof(ListAllocator),
	       (PAGESIZE * 4) - sizeof(ListAllocator));

    /* Set default allocator. */
    Allocator::setDefault(li);

    /* Pass control to the program. */
    main(1, argv);
}
