/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-candidate.h
 * This file is part of AIM.
 *
 * Copyright (C) 2015 Hodong Kim <hodong@cogno.org>
 *
 * AIM is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AIM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program;  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __AIM_CANDIDATE_H__
#define __AIM_CANDIDATE_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <glib-object.h>
#include "aim-connection.h"

G_BEGIN_DECLS

#define AIM_TYPE_CANDIDATE             (aim_candidate_get_type ())
#define AIM_CANDIDATE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_TYPE_CANDIDATE, AimCandidate))
#define AIM_CANDIDATE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AIM_TYPE_CANDIDATE, AimCandidateClass))
#define AIM_IS_CANDIDATE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AIM_TYPE_CANDIDATE))
#define AIM_IS_CANDIDATE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AIM_TYPE_CANDIDATE))
#define AIM_CANDIDATE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AIM_TYPE_CANDIDATE, AimCandidateClass))

typedef struct _AimConnection      AimConnection;

typedef struct _AimCandidate       AimCandidate;
typedef struct _AimCandidateClass  AimCandidateClass;

GType         aim_candidate_get_type (void) G_GNUC_CONST;

AimCandidate *aim_candidate_new                   (void);
void          aim_candidate_update_window         (AimCandidate  *candidate,
                                                   const gchar  **strv);
void          aim_candidate_show_window           (AimCandidate  *candidate,
                                                   AimConnection *target);
void          aim_candidate_hide_window           (AimCandidate  *candidate);
void          aim_candidate_select_previous_item  (AimCandidate  *candidate);
void          aim_candidate_select_next_item      (AimCandidate  *candidate);
void          aim_candidate_select_page_up_item   (AimCandidate  *candidate);
void          aim_candidate_select_page_down_item (AimCandidate  *candidate);
gchar        *aim_candidate_get_selected_text     (AimCandidate  *candidate);

G_END_DECLS

#endif /* __AIM_CANDIDATE_H__ */

