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

#include "c-spiffe/bundle/spiffebundle/source.h"
#include "c-spiffe/federation/watcher.h"
#include "c-spiffe/spiffeid/trustdomain.h"
#include <check.h>
#include <unistd.h>

int running = true;

int print_watcher(void *watcher_)
{
    spiffebundle_Watcher *watcher = (spiffebundle_Watcher *) watcher_;
    // waits and prints bundle every few seconds
    struct timespec update_time = { .tv_sec = 2, .tv_nsec = 0 };

    while(running) {
        err_t err = NO_ERROR;
        nanosleep(&update_time, NULL);

        for(size_t i = 0, size = shlenu(watcher->endpoints); i < size; ++i) {
            spiffebundle_Endpoint_Status *status = watcher->endpoints[i].value;
            if(!status) {
                fprintf(stderr, "Error .\n");
                continue;
            }
            printf("SPIFFE BUNDLE:\nTrust Domain: \"%s\"\n",
                   status->endpoint->td.name);
            spiffebundle_Bundle *bundle
                = spiffebundle_Source_GetSpiffeBundleForTrustDomain(
                    status->endpoint->source, status->endpoint->td, &err);
            if(!bundle) {
                fprintf(stderr, "Error retrieving bundle from source.\n");
                printf("Can't find a bundle for \"%s\".\n",
                       status->endpoint->td.name);
                continue;
            }
            string_t m_string = spiffebundle_Bundle_Marshal(bundle, &err);
            if(m_string) {
                printf("Bundle (jwks):\n%s\n", m_string);
                util_string_t_Free(m_string);
            } else {
                printf("Bundle (jwks): ERROR\n");
                fprintf(stderr, "Error marshalling bundle.\n");
            }
        }
    }
}

int main(void)
{
    // create watcher
    spiffebundle_Watcher *watcher = spiffebundle_Watcher_New();
    err_t err;

    // endpoint info
    const char url[] = "https://example.org:443/";
    spiffeid_TrustDomain td = { "example.org" };
    spiffebundle_Bundle *initial_bundle
        = spiffebundle_Load(td, "./resources/example.org.bundle.jwks", &err);
    if(err != NO_ERROR) {
        printf("ERROR %d loading bundle\n", err);
    }
    spiffebundle_Source *source
        = spiffebundle_SourceFromBundle(initial_bundle);
    // set up an endpoint
    printf("Adding Endpoint\n");
    err = spiffebundle_Watcher_AddHttpsSpiffeEndpoint(
        watcher, url, td, "spiffe://example.org/workload", source);

    // add any more endpoints you want
    // err = spiffebundle_Watcher_AddHttpsWebEndpoint(watcher, url2, td2);
    // err = spiffebundle_Watcher_AddHttpsSpiffeEndpoint(watcher, url3, td3,
    // id, source);
    if(err != NO_ERROR) {
        // free memory and exit
        fprintf(stderr, "\nERROR ADDING ENDPOINT: %d\n", err);
        spiffebundle_Watcher_Free(watcher);
        exit(err);
    }
    // starts watcher up, will not block
    printf("Starting Watcher.\n");
    err = spiffebundle_Watcher_Start(watcher);

    if(err != NO_ERROR) {
        // free memory and exit
        fprintf(stderr, "\nERROR STARTING WATCHER: %d\n", err);
        spiffebundle_Watcher_Free(watcher);
        exit(err);
    }

    // start threads in the background
    printf("Starting printing thread.\n");

    thrd_t printer;
    int thread_ret = thrd_create(&printer, print_watcher, watcher);
    if(thread_ret != thrd_success) {
        // free memory and exit
        fprintf(stderr, "\nERROR STARTING THREAD: %d\n", err);
        spiffebundle_Watcher_Free(watcher);
        exit(err);
    }
    // wait until ENTER is pressed
    printf("Press ENTER to stop.\n");
    char ch;
    scanf("%c", &ch);
    running = false;

    // stop watcher, blocks waiting for threads to close
    err = spiffebundle_Watcher_Stop(watcher);
    printf("Stopping Watcher.\n");
    if(err != NO_ERROR) {
        // free memory and exit
        fprintf(stderr, "\nERROR STOPPING WATCHER: %d\n", err);
        spiffebundle_Watcher_Free(watcher);
        exit(err);
    }

    // free memory
    spiffebundle_Watcher_Free(watcher);
    return 0;
}
