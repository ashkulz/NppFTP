/*
 * poll.c - poll wrapper
 *
 * This file is part of the SSH Library
 *
 * Copyright (c) 2003-2009 by Aris Adamantiadis
 * Copyright (c) 2009 Aleksandar Kanchev
 *
 * The SSH Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * The SSH Library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the SSH Library; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 * vim: ts=2 sw=2 et cindent
 */

/* This code is based on glib's gpoll */

#include "config.h"

#include <errno.h>

#include "libssh/priv.h"
#include "libssh/libssh.h"
#include "libssh/poll.h"

#ifndef SSH_POLL_CTX_CHUNK
#define SSH_POLL_CTX_CHUNK			5
#endif

struct ssh_poll_handle_struct {
  ssh_poll_ctx ctx;
  union {
    socket_t fd;
    size_t idx;
  } x;
  short events;
  ssh_poll_callback cb;
  void *cb_data;
};

struct ssh_poll_ctx_struct {
  ssh_poll_handle *pollptrs;
  ssh_pollfd_t *pollfds;
  size_t polls_allocated;
  size_t polls_used;
  size_t chunk_size;
};

#ifdef HAVE_POLL
#include <poll.h>

int ssh_poll(ssh_pollfd_t *fds, nfds_t nfds, int timeout) {
  return poll((struct pollfd *) fds, nfds, timeout);
}

#else /* HAVE_POLL */
#ifdef _WIN32

#if 0
/* defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0600) */

#include <winsock2.h>

int ssh_poll(ssh_pollfd_t *fds, nfds_t nfds, int timeout) {
  return WSAPoll(fds, nfds, timeout);
}

#else /* _WIN32_WINNT */

#ifndef STRICT
#define STRICT
#endif

#include <stdio.h>
#include <windows.h>
#include <errno.h>

static int poll_rest (HANDLE *handles, int nhandles,
    ssh_pollfd_t *fds, nfds_t nfds, int timeout) {
  DWORD ready;
  ssh_pollfd_t *f;
  int recursed_result;

  if (nhandles == 0) {
    /* No handles to wait for, just the timeout */
    if (timeout == INFINITE) {
      ready = WAIT_FAILED;
    } else {
      SleepEx(timeout, 1);
      ready = WAIT_TIMEOUT;
    }
  } else {
    /* Wait for just handles */
    ready = WaitForMultipleObjectsEx(nhandles, handles, FALSE, timeout, TRUE);
#if 0
    if (ready == WAIT_FAILED)  {
      fprintf(stderr, "WaitForMultipleObjectsEx failed: %d\n", GetLastError());
    }
#endif
  }

  if (ready == WAIT_FAILED) {
    return -1;
  } else if (ready == WAIT_TIMEOUT || ready == WAIT_IO_COMPLETION) {
    return 0;
  } else if (ready >= WAIT_OBJECT_0 && ready < WAIT_OBJECT_0 + nhandles) {
    for (f = fds; f < &fds[nfds]; f++) {
      if ((HANDLE) f->fd == handles[ready - WAIT_OBJECT_0]) {
        f->revents = f->events;
      }
    }

    /*
     * If no timeout and polling several handles, recurse to poll
     * the rest of them.
     */
    if (timeout == 0 && nhandles > 1) {
      /* Remove the handle that fired */
      int i;
      if (ready < nhandles - 1) {
        for (i = ready - WAIT_OBJECT_0 + 1; i < nhandles; i++) {
          handles[i-1] = handles[i];
        }
      }
      nhandles--;
      recursed_result = poll_rest(handles, nhandles, fds, nfds, 0);
      if (recursed_result < 0) {
        return -1;
      }
      return recursed_result + 1;
    }
    return 1;
  }

  return 0;
}

int ssh_poll(ssh_pollfd_t *fds, nfds_t nfds, int timeout) {
  HANDLE handles[MAXIMUM_WAIT_OBJECTS];
  ssh_pollfd_t *f;
  int nhandles = 0;
  int rc = -1;

  if (fds == NULL) {
    errno = EFAULT;
    return -1;
  }

  if (nfds >= MAXIMUM_WAIT_OBJECTS) {
    errno = EINVAL;
    return -1;
  }

  for (f = fds; f < &fds[nfds]; f++) {
    if (f->fd > 0) {
      int i;

      /*
       * Don't add the same handle several times into the array, as
       * docs say that is not allowed, even if it actually does seem
       * to work.
       */
      for (i = 0; i < nhandles; i++) {
        if (handles[i] == (HANDLE) f->fd) {
          break;
        }
      }

      if (i == nhandles) {
        if (nhandles == MAXIMUM_WAIT_OBJECTS) {
          break;
        } else {
          handles[nhandles++] = (HANDLE) f->fd;
        }
      }
    }
  }

  if (timeout == -1) {
    timeout = INFINITE;
  }

  if (nhandles > 1) {
    /*
     * First check if one or several of them are immediately
     * available.
     */
    rc = poll_rest(handles, nhandles, fds, nfds, 0);

    /*
     * If not, and we have a significant timeout, poll again with
     * timeout then. Note that this will return indication for only
     * one event, or only for messages. We ignore timeouts less than
     * ten milliseconds as they are mostly pointless on Windows, the
     * MsgWaitForMultipleObjectsEx() call will timeout right away
     * anyway.
     */
    if (rc == 0 && (timeout == INFINITE || timeout >= 10)) {
      rc = poll_rest(handles, nhandles, fds, nfds, timeout);
    }
  } else {
    /*
     * Just polling for one thing, so no need to check first if
     * available immediately
     */
    rc = poll_rest(handles, nhandles, fds, nfds, timeout);
  }

  if (rc < 0) {
    for (f = fds; f < &fds[nfds]; f++) {
      f->revents = 0;
    }
    errno = EBADF;
  }

  return rc;
}

#endif /* _WIN32_WINNT */

#endif /* _WIN32 */

#endif /* HAVE_POLL */

/**
 * @brief  Allocate a new poll object, which could be used within a poll context.
 *
 * @param  fd           Socket that will be polled.
 * @param  events       Poll events that will be monitored for the socket. i.e.
 *                      POLLIN, POLLPRI, POLLOUT, POLLERR, POLLHUP, POLLNVAL
 * @param  cb           Function to be called if any of the events are set.
 * @param  userdata     Userdata to be passed to the callback function. NULL if
 *                      not needed.
 *
 * @return              A new poll object, NULL on error
 */

ssh_poll_handle ssh_poll_new(socket_t fd, short events, ssh_poll_callback cb,
    void *userdata) {
  ssh_poll_handle p;

  p = malloc(sizeof(struct ssh_poll_handle_struct));
  if (p != NULL) {
    p->ctx = NULL;
    p->x.fd = fd;
    p->events = events;
    p->cb = cb;
    p->cb_data = userdata;
  }

  return p;
}


/**
 * @brief  Free a poll object.
 *
 * @param  p            Pointer to an already allocated poll object.
 */

void ssh_poll_free(ssh_poll_handle p) {
  SAFE_FREE(p);
}

/**
 * @brief  Get the poll context of a poll object.
 *
 * @param  p            Pointer to an already allocated poll object.
 *
 * @return              Poll context or NULL if the poll object isn't attached.
 */
ssh_poll_ctx ssh_poll_get_ctx(ssh_poll_handle p) {
  return p->ctx;
}

/**
 * @brief  Get the events of a poll object.
 *
 * @param  p            Pointer to an already allocated poll object.
 *
 * @return              Poll events.
 */
short ssh_poll_get_events(ssh_poll_handle p) {
  return p->events;
}

/**
 * @brief  Set the events of a poll object. The events will also be propagated
 *         to an associated poll context.
 *
 * @param  p            Pointer to an already allocated poll object.
 * @param  events       Poll events.
 */
void ssh_poll_set_events(ssh_poll_handle p, short events) {
  p->events = events;
  if (p->ctx != NULL) {
    p->ctx->pollfds[p->x.idx].events = events;
  }
}

/**
 * @brief  Add extra events to a poll object. Duplicates are ignored.
 *         The events will also be propagated to an associated poll context.
 *
 * @param  p            Pointer to an already allocated poll object.
 * @param  events       Poll events.
 */
void ssh_poll_add_events(ssh_poll_handle p, short events) {
  ssh_poll_set_events(p, ssh_poll_get_events(p) | events);
}

/**
 * @brief  Remove events from a poll object. Non-existent are ignored.
 *         The events will also be propagated to an associated poll context.
 *
 * @param  p            Pointer to an already allocated poll object.
 * @param  events       Poll events.
 */
void ssh_poll_remove_events(ssh_poll_handle p, short events) {
  ssh_poll_set_events(p, ssh_poll_get_events(p) & ~events);
}

/**
 * @brief  Get the raw socket of a poll object.
 *
 * @param  p            Pointer to an already allocated poll object.
 *
 * @return              Raw socket.
 */

socket_t ssh_poll_get_fd(ssh_poll_handle p) {
  if (p->ctx != NULL) {
    return p->ctx->pollfds[p->x.idx].fd;
  }

  return p->x.fd;
}
/**
 * @brief  Set the callback of a poll object.
 *
 * @param  p            Pointer to an already allocated poll object.
 * @param  cb           Function to be called if any of the events are set.
 * @param  userdata     Userdata to be passed to the callback function. NULL if
 *                      not needed.
 */
void ssh_poll_set_callback(ssh_poll_handle p, ssh_poll_callback cb, void *userdata) {
  if (cb != NULL) {
    p->cb = cb;
    p->cb_data = userdata;
  }
}

/**
 * @brief  Create a new poll context. It could be associated with many poll object
 *         which are going to be polled at the same time as the poll context. You
 *         would need a single poll context per thread.
 *
 * @param  chunk_size   The size of the memory chunk that will be allocated, when
 *                      more memory is needed. This is for efficiency reasons,
 *                      i.e. don't allocate memory for each new poll object, but
 *                      for the next 5. Set it to 0 if you want to use the
 *                      library's default value.
 */
ssh_poll_ctx ssh_poll_ctx_new(size_t chunk_size) {
  ssh_poll_ctx ctx;

  ctx = malloc(sizeof(struct ssh_poll_ctx_struct));
  if (ctx != NULL) {
    if (!chunk_size) {
      chunk_size = SSH_POLL_CTX_CHUNK;
    }

    ctx->chunk_size = chunk_size;
    ctx->pollptrs = NULL;
    ctx->pollfds = NULL;
    ctx->polls_allocated = 0;
    ctx->polls_used = 0;
  }

  return ctx;
}

/**
 * @brief  Free a poll context.
 *
 * @param  ctx          Pointer to an already allocated poll context.
 */
void ssh_poll_ctx_free(ssh_poll_ctx ctx) {
  if (ctx->polls_allocated > 0) {
    register size_t i, used;

    used = ctx->polls_used;
    for (i = 0; i < used; ) {
      ssh_poll_handle p = ctx->pollptrs[i];
      int fd = ctx->pollfds[i].fd;

      /* force poll object removal */
      if (p->cb(p, fd, POLLERR, p->cb_data) < 0) {
        used = ctx->polls_used;
      } else {
        i++;
      }
    }

    SAFE_FREE(ctx->pollptrs);
    SAFE_FREE(ctx->pollfds);
  }

  SAFE_FREE(ctx);
}

static int ssh_poll_ctx_resize(ssh_poll_ctx ctx, size_t new_size) {
  ssh_poll_handle *pollptrs;
  ssh_pollfd_t *pollfds;

  pollptrs = realloc(ctx->pollptrs, sizeof(ssh_poll_handle *) * new_size);
  if (pollptrs == NULL) {
    return -1;
  }

  pollfds = realloc(ctx->pollfds, sizeof(ssh_pollfd_t) * new_size);
  if (pollfds == NULL) {
    ctx->pollptrs = realloc(pollptrs, sizeof(ssh_poll_handle *) * ctx->polls_allocated);
    return -1;
  }

  ctx->pollptrs = pollptrs;
  ctx->pollfds = pollfds;
  ctx->polls_allocated = new_size;

  return 0;
}

/**
 * @brief  Add a poll object to a poll context.
 *
 * @param  ctx          Pointer to an already allocated poll context.
 * @param  p            Pointer to an already allocated poll object.
 *
 * @return              0 on success, < 0 on error
 */
int ssh_poll_ctx_add(ssh_poll_ctx ctx, ssh_poll_handle p) {
  int fd;

  if (p->ctx != NULL) {
    /* already attached to a context */
    return -1;
  }

  if (ctx->polls_used == ctx->polls_allocated &&
      ssh_poll_ctx_resize(ctx, ctx->polls_allocated + ctx->chunk_size) < 0) {
    return -1;
  }

  fd = p->x.fd;
  p->x.idx = ctx->polls_used++;
  ctx->pollptrs[p->x.idx] = p;
  ctx->pollfds[p->x.idx].fd = fd;
  ctx->pollfds[p->x.idx].events = p->events;
  ctx->pollfds[p->x.idx].revents = 0;
  p->ctx = ctx;

  return 0;
}

/**
 * @brief  Remove a poll object from a poll context.
 *
 * @param  ctx          Pointer to an already allocated poll context.
 * @param  p            Pointer to an already allocated poll object.
 */
void ssh_poll_ctx_remove(ssh_poll_ctx ctx, ssh_poll_handle p) {
  size_t i;

  i = p->x.idx;
  p->x.fd = ctx->pollfds[i].fd;
  p->ctx = NULL;

  ctx->polls_used--;

  /* fill the empty poll slot with the last one */
  if (ctx->polls_used > 0 && ctx->polls_used != i) {
    ctx->pollfds[i] = ctx->pollfds[ctx->polls_used];
    ctx->pollptrs[i] = ctx->pollptrs[ctx->polls_used];
  }

  /* this will always leave at least chunk_size polls allocated */
  if (ctx->polls_allocated - ctx->polls_used > ctx->chunk_size) {
    ssh_poll_ctx_resize(ctx, ctx->polls_allocated - ctx->chunk_size);
  }
}

/**
 * @brief  Poll all the sockets associated through a poll object with a
 *         poll context. If any of the events are set after the poll, the
 *         call back function of the socket will be called.
 *         This function should be called once within the programs main loop.
 *
 * @param  ctx          Pointer to an already allocated poll context.
 * @param  timeout      An upper limit on the time for which ssh_poll_ctx() will
 *                      block, in milliseconds. Specifying a negative value
 *                      means an infinite timeout. This parameter is passed to
 *                      the poll() function.
 */

int ssh_poll_ctx_dopoll(ssh_poll_ctx ctx, int timeout) {
  int rc;

  if (!ctx->polls_used)
    return 0;

  rc = ssh_poll(ctx->pollfds, ctx->polls_used, timeout);
  if (rc > 0) {
    register size_t i, used;

    used = ctx->polls_used;
    for (i = 0; i < used && rc > 0; ) {
      if (!ctx->pollfds[i].revents) {
        i++;
      } else {
        ssh_poll_handle p = ctx->pollptrs[i];
        int fd = ctx->pollfds[i].fd;
        int revents = ctx->pollfds[i].revents;

        if (p->cb(p, fd, revents, p->cb_data) < 0) {
          /* the poll was removed, reload the used counter and stall the loop */
          used = ctx->polls_used;
        } else {
          ctx->pollfds[i].revents = 0;
          i++;
        }

        rc--;
      }
    }
  }

  return rc;
}
