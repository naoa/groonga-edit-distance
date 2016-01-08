/* Copyright(C) 2016 Naoya Murakami <naoya@createfield.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301  USA
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <groonga/plugin.h>

#ifdef __GNUC__
# define GNUC_UNUSED __attribute__((__unused__))
#else
# define GNUC_UNUSED
#endif

#define DIST(ox,oy) (dists[((lx + 1) * (oy)) + (ox)])

static grn_obj *
func_damerau_edit_distance(grn_ctx *ctx, int nargs, grn_obj **args, grn_user_data *user_data)
{
#define N_REQUIRED_ARGS 2
#define MAX_ARGS 3
  double d = 0;
  grn_obj *obj;
  if (nargs >= N_REQUIRED_ARGS && nargs <= MAX_ARGS) {
    uint32_t cx, lx, cy, ly;
    double *dists;
    char *px, *sx = GRN_TEXT_VALUE(args[0]), *ex = GRN_BULK_CURR(args[0]);
    char *py, *sy = GRN_TEXT_VALUE(args[1]), *ey = GRN_BULK_CURR(args[1]);
    unsigned int deletion_cost = 1;
    unsigned int insertion_cost = 1;
    unsigned int substitution_cost = 1;
    unsigned int transposition_cost = 1;
    grn_bool with_transposition = GRN_TRUE;
    if (nargs == 3) {
      with_transposition = GRN_BOOL_VALUE(args[2]);
    }
    for (px = sx, lx = 0; px < ex && (cx = grn_charlen(ctx, px, ex)); px += cx, lx++);
    for (py = sy, ly = 0; py < ey && (cy = grn_charlen(ctx, py, ey)); py += cy, ly++);
    if ((dists = GRN_PLUGIN_MALLOC(ctx, (lx + 1) * (ly + 1) * sizeof(double)))) {
      uint32_t x, y;
      for (x = 0; x <= lx; x++) { DIST(x, 0) = x; }
      for (y = 0; y <= ly; y++) { DIST(0, y) = y; }
      for (x = 1, px = sx; x <= lx; x++, px += cx) {
        cx = grn_charlen(ctx, px, ex);
        for (y = 1, py = sy; y <= ly; y++, py += cy) {
          cy = grn_charlen(ctx, py, ey);
          if (cx == cy && !memcmp(px, py, cx)) {
            DIST(x, y) = DIST(x - 1, y - 1);
          } else {
            double a;
            double b;
            double c;
            a = DIST(x - 1, y) + deletion_cost;
            b = DIST(x, y - 1) + insertion_cost;
            c = DIST(x - 1, y - 1) + substitution_cost;
            DIST(x, y) = ((a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c));
            if (with_transposition && x > 1 && y > 1
                && cx == cy
                && memcmp(px, py - cy, cx) == 0
                && memcmp(px - cx, py, cx) == 0) {
              double t = DIST(x - 2, y - 2) + transposition_cost;
              DIST(x, y) = ((DIST(x, y) < t) ? DIST(x, y) : t);
            }
          }
        }
      }
      d = DIST(lx, ly);
      GRN_PLUGIN_FREE(ctx, dists);
    }
  }
  if ((obj = grn_plugin_proc_alloc(ctx, user_data, GRN_DB_FLOAT, 0))) {
    GRN_FLOAT_SET(ctx, obj, d);
  }
  return obj;
#undef N_REQUIRED_ARGS
#undef MAX_ARGS
}

/* func_edit_distance_bp() is only surported to 1byte char */
static grn_obj *
func_edit_distance_bp(grn_ctx *ctx, int nargs, grn_obj **args, grn_user_data *user_data)
{
#define N_REQUIRED_ARGS 2
#define MAX_ARGS 3
#define MAX_WORD_SIZE 64
  uint32_t score = 0;
  grn_obj *obj = NULL;
  if (nargs >= N_REQUIRED_ARGS && nargs <= MAX_ARGS) {
    unsigned int i, j;
    const unsigned char *search;
    unsigned int search_length;
    const unsigned char *compared;
    unsigned int compared_length;
    uint64_t char_vector[UCHAR_MAX] = {0};
    uint64_t top;
    uint64_t VP = 0xFFFFFFFFFFFFFFFFULL;
    uint64_t VN = 0;
    grn_bool with_transposition = GRN_TRUE;
    if (nargs == MAX_ARGS) {
      with_transposition = GRN_BOOL_VALUE(args[2]);
    }
    if (GRN_TEXT_LEN(args[0]) > MAX_WORD_SIZE || GRN_TEXT_LEN(args[1]) > MAX_WORD_SIZE) {
      return func_damerau_edit_distance(ctx, nargs, args, user_data);
    } else if (GRN_TEXT_LEN(args[0]) >= GRN_TEXT_LEN(args[1])) {
      search = (const unsigned char *)GRN_TEXT_VALUE(args[0]);
      search_length = GRN_TEXT_LEN(args[0]);
      compared = (const unsigned char *)GRN_TEXT_VALUE(args[1]);
      compared_length = GRN_TEXT_LEN(args[1]);
    } else {
      search = (const unsigned char *)GRN_TEXT_VALUE(args[1]);
      search_length = GRN_TEXT_LEN(args[1]);
      compared = (const unsigned char *)GRN_TEXT_VALUE(args[0]);
      compared_length = GRN_TEXT_LEN(args[0]);
    }

    top = (1ULL << (search_length - 1));
    score = search_length;
    for (i = 0; i < search_length; i++) {
      char_vector[search[i]] |= (1ULL << i);
    }
    for (j = 0; j < compared_length; j++) {
      uint64_t D0 = 0, HP, HN;
      if (with_transposition) {
        uint64_t PM[2];
        if (j > 0) {
          PM[0] = char_vector[compared[j - 1]];
        } else {
          PM[0] = 0;
        }
        PM[1] = char_vector[compared[j]];
        D0 = ((( ~ D0) & PM[1]) << 1ULL) & PM[0];
        D0 = D0 | (((PM[1] & VP) + VP) ^ VP) | PM[1] | VN;
      } else {
        uint64_t PM;
        PM = char_vector[compared[j]];
        D0 = (((PM & VP) + VP) ^ VP) | PM | VN;
      }
      HP = VN | ~(D0 | VP);
      HN = VP & D0;
      if (HP & top) {
        score++;
      } else if (HN & top) {
        score--;
      }
      VP = (HN << 1ULL) | ~(D0 | ((HP << 1ULL) | 1ULL));
      VN = D0 & ((HP << 1ULL) | 1ULL);
    }
  }
  if ((obj = grn_plugin_proc_alloc(ctx, user_data, GRN_DB_FLOAT, 0))) {
    GRN_FLOAT_SET(ctx, obj, score);
  }
  return obj;
#undef N_REQUIRED_ARGS
#undef MAX_ARGS
#undef MAX_WORD_SIZE
}

static grn_obj *
func_edit_distance_bp_var(grn_ctx *ctx, int nargs, grn_obj **args, grn_user_data *user_data)
{
#define N_REQUIRED_ARGS 2
#define MAX_ARGS 4
#define MAX_WORD_SIZE 64
  uint32_t score = 0;
  grn_obj *obj;
  grn_obj *table_ptr = NULL;
  grn_obj *table = NULL;
  grn_obj *search_ids_ptr = NULL;
  grn_obj *compared_ids_ptr = NULL;
  grn_bool use_cache = GRN_TRUE;

  if (nargs >= N_REQUIRED_ARGS && nargs <= MAX_ARGS) {
    unsigned int i, j;
    const char *search = GRN_TEXT_VALUE(args[0]);
    const char *search_end = GRN_BULK_CURR(args[0]);
    const char *compared = GRN_TEXT_VALUE(args[1]);
    const char *compared_end = GRN_BULK_CURR(args[1]);
    grn_obj search_ids;
    grn_obj compared_ids;
    unsigned int n_search_ids;
    unsigned int n_compared_ids;
    int char_length;
    uint64_t char_vector[128] = {0};
    uint64_t top;
    uint64_t VP = 0xFFFFFFFFFFFFFFFFULL;
    uint64_t VN = 0;
    grn_bool with_transposition = GRN_FALSE;
    if (nargs >= 3) {
      with_transposition = GRN_BOOL_VALUE(args[2]);
    }
    if (nargs == 4) {
      use_cache = GRN_BOOL_VALUE(args[3]);
    }

    if (!use_cache) {
      table = grn_table_create(ctx, NULL, 0, NULL,
                               GRN_TABLE_HASH_KEY,
                               grn_ctx_at(ctx, GRN_DB_SHORT_TEXT), NULL);

      GRN_UINT32_INIT(&search_ids, GRN_OBJ_VECTOR);
      search_ids_ptr = &search_ids;

      for (i = 0; (char_length = grn_charlen(ctx, search, search_end)) > 0; i++) {
        grn_id id = grn_table_add(ctx, table, search, char_length, NULL);
        grn_uvector_add_element(ctx, search_ids_ptr, id - 1, 0);
        search += char_length;
      }
      n_search_ids = i;
    } else {
      grn_obj *expression = NULL;
      grn_proc_get_info(ctx, user_data, NULL, NULL, &expression);
      table_ptr = grn_expr_get_var(ctx, expression,
                                   "$edit_distance_table",
                                   strlen("$edit_distance_table"));
      search_ids_ptr = grn_expr_get_var(ctx, expression,
                                   "$edit_distance_ids",
                                   strlen("$edit_distance_ids"));
      if (table_ptr && search_ids_ptr) {
        table = GRN_PTR_VALUE(table_ptr);
        n_search_ids = grn_uvector_size(ctx, search_ids_ptr);
      } else {
        table_ptr = grn_expr_add_var(ctx, expression,
                                    "$edit_distance_table",
                                    strlen("$edit_distance_table"));

        GRN_OBJ_FIN(ctx, table_ptr);
        GRN_PTR_INIT(table_ptr, GRN_OBJ_OWN, GRN_DB_OBJECT);

        table = grn_table_create(ctx, NULL, 0, NULL,
                                 GRN_TABLE_HASH_KEY,
                                 grn_ctx_at(ctx, GRN_DB_SHORT_TEXT), NULL);
        GRN_PTR_SET(ctx, table_ptr, table);

        search_ids_ptr = grn_expr_add_var(ctx, expression,
                                          "$edit_distance_ids",
                                          strlen("$edit_distance_ids"));
        GRN_UINT32_INIT(search_ids_ptr, GRN_OBJ_VECTOR);
        for (i = 0; (char_length = grn_charlen(ctx, search, search_end)) > 0; i++) {
          grn_id id = grn_table_add(ctx, table, search, char_length, NULL);
          grn_uvector_add_element(ctx, search_ids_ptr, id - 1, 0);
          search += char_length;
        }
        n_search_ids = i;
      }
    }

    if (n_search_ids > MAX_WORD_SIZE) {
      if (!use_cache) {
        grn_obj_unlink(ctx, &search_ids);
      }
      return func_damerau_edit_distance(ctx, nargs, args, user_data);
    }

    GRN_UINT32_INIT(&compared_ids, GRN_OBJ_VECTOR);
    for (i = 0; (char_length = grn_charlen(ctx, compared, compared_end)) > 0; i++) {
      grn_id id = grn_table_add(ctx, table, compared, char_length, NULL);
      grn_uvector_add_element(ctx, &compared_ids, id - 1, 0);
      compared += char_length;
    }
    n_compared_ids = i;

    if (n_compared_ids > MAX_WORD_SIZE) {
      grn_obj_unlink(ctx, &compared_ids);
      return func_damerau_edit_distance(ctx, nargs, args, user_data);
    }

    if (n_search_ids >= n_compared_ids) {
      for (i = 0; i < n_search_ids; i++) {
        char_vector[grn_uvector_get_element(ctx, search_ids_ptr, i, 0)] |= (1ULL << i);
      }
      top = (1ULL << (n_search_ids - 1));
      score = n_search_ids;
      compared_ids_ptr = &compared_ids;
    } else {
      for (i = 0; i < n_compared_ids; i++) {
        char_vector[grn_uvector_get_element(ctx, &compared_ids, i, 0)] |= (1ULL << i);
      }
      top = (1ULL << (n_compared_ids - 1));
      score = n_compared_ids;
      n_compared_ids = n_search_ids;
      compared_ids_ptr = search_ids_ptr;
    }

    for (j = 0; j < n_compared_ids; j++) {
      uint64_t D0 = 0, HP, HN;
      if (with_transposition) {
        uint64_t PM[2];
        if (j > 0) {
          PM[0] = char_vector[grn_uvector_get_element(ctx, compared_ids_ptr, j - 1, 0)];
        } else {
          PM[0] = 0;
        }
        PM[1] = char_vector[grn_uvector_get_element(ctx, compared_ids_ptr, j, 0)];
        D0 = ((( ~ D0) & PM[1]) << 1ULL) & PM[0];
        D0 = D0 | (((PM[1] & VP) + VP) ^ VP) | PM[1] | VN;
      } else {
        uint64_t PM;
        PM = char_vector[grn_uvector_get_element(ctx, compared_ids_ptr, j, 0)];
        D0 = (((PM & VP) + VP) ^ VP) | PM | VN;
      }
      HP = VN | ~(D0 | VP);
      HN = VP & D0;
      if (HP & top) {
        score++;
      } else if (HN & top) {
        score--;
      }
      VP = (HN << 1ULL) | ~(D0 | ((HP << 1ULL) | 1ULL));
      VN = D0 & ((HP << 1ULL) | 1ULL);
    }
    if (!use_cache) {
      grn_obj_unlink(ctx, &search_ids);
    }
    grn_obj_unlink(ctx, &compared_ids);
  }

  if ((obj = grn_plugin_proc_alloc(ctx, user_data, GRN_DB_FLOAT, 0))) {
    GRN_FLOAT_SET(ctx, obj, score);
  }
  return obj;
#undef N_REQUIRED_ARGS
#undef MAX_ARGS
#undef MAX_WORD_SIZE
}

grn_rc
GRN_PLUGIN_INIT(GNUC_UNUSED grn_ctx *ctx)
{
  return GRN_SUCCESS;
}

grn_rc
GRN_PLUGIN_REGISTER(grn_ctx *ctx)
{
  grn_proc_create(ctx, "damerau_edit_distance", -1, GRN_PROC_FUNCTION,
                  func_damerau_edit_distance, NULL, NULL, 0, NULL);

  grn_proc_create(ctx, "edit_distance_bp", -1, GRN_PROC_FUNCTION,
                  func_edit_distance_bp, NULL, NULL, 0, NULL);

  grn_proc_create(ctx, "edit_distance_bp_var", -1, GRN_PROC_FUNCTION,
                  func_edit_distance_bp_var, NULL, NULL, 0, NULL);

  return ctx->rc;
}

grn_rc
GRN_PLUGIN_FIN(GNUC_UNUSED grn_ctx *ctx)
{
  return GRN_SUCCESS;
}
