/**
 *
 * (C) Copyright 2020-2021 Hewlett Packard Enterprise Development LP
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 */

#include "c-spiffe/bundle/jwtbundle/source.h"

jwtbundle_Bundle *jwtbundle_Source_GetJWTBundleForTrustDomain(
    jwtbundle_Source *s, const spiffeid_TrustDomain td, err_t *err)
{
    if(s->type == JWTBUNDLE_BUNDLE) {
        return jwtbundle_Bundle_GetJWTBundleForTrustDomain(s->source.bundle,
                                                           td, err);
    } else if(s->type == JWTBUNDLE_SET) {
        return jwtbundle_Set_GetJWTBundleForTrustDomain(s->source.set, td,
                                                        err);
    } else if(s->type == JWTBUNDLE_WORKLOADAPI_JWTSOURCE) {
        return workloadapi_JWTSource_GetJWTBundleForTrustDomain(
            s->source.source, td, err);
    }

    return NULL;
}

jwtbundle_Source *jwtbundle_SourceFromBundle(jwtbundle_Bundle *b)
{
    if(b) {
        jwtbundle_Source *source = malloc(sizeof *source);

        source->type = JWTBUNDLE_BUNDLE;
        source->source.bundle = b;

        return source;
    }

    return NULL;
}

jwtbundle_Source *jwtbundle_SourceFromSet(jwtbundle_Set *s)
{
    if(s) {
        jwtbundle_Source *source = malloc(sizeof *source);

        source->type = JWTBUNDLE_SET;
        source->source.set = s;

        return source;
    }

    return NULL;
}

jwtbundle_Source *jwtbundle_SourceFromSource(workloadapi_JWTSource *s)
{
    if(s) {
        jwtbundle_Source *source = malloc(sizeof *source);

        source->type = JWTBUNDLE_WORKLOADAPI_JWTSOURCE;
        source->source.source = s;

        return source;
    }

    return NULL;
}

void jwtbundle_Source_Free(jwtbundle_Source *s)
{
    if(s) {
        if(s->type == JWTBUNDLE_BUNDLE) {
            jwtbundle_Bundle_Free(s->source.bundle);
        } else if(s->type == JWTBUNDLE_SET) {
            jwtbundle_Set_Free(s->source.set);
        } else if(s->type == JWTBUNDLE_WORKLOADAPI_JWTSOURCE) {
            workloadapi_JWTSource_Free(s->source.source);
        }

        free(s);
    }
}
