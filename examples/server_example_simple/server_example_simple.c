/*
 *  server_example_simple.c
 *
 *  Copyright 2013 Michael Zillgith
 *
 *	This file is part of libIEC61850.
 *
 *	libIEC61850 is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	libIEC61850 is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with libIEC61850.  If not, see <http://www.gnu.org/licenses/>.
 *
 *	See COPYING file for the complete license text.
 */

#include "iec61850_server.h"
#include "hal_thread.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "static_model.h"
#include "mms_value.h"
#include "mms_value_internal.h"

/* import IEC 61850 device model created from SCL-File */
extern IedModel iedModel;

static int running = 0;

void sigint_handler(int signalId)
{
	running = 0;
}

int main(int argc, char** argv) {

    int tcpPort = 102;

    if (argc > 1) {
        tcpPort = atoi(argv[1]);
    }

	IedServer iedServer = IedServer_create(&iedModel);

	// set initial measurement and status values from process

	/* MMS server will be instructed to start listening to client connections. */
	IedServer_start(iedServer, tcpPort);

	if (!IedServer_isRunning(iedServer)) {
		printf("Starting server failed! Exit.\n");
		IedServer_destroy(iedServer);
		exit(-1);
	}

	running = 1;

	signal(SIGINT, sigint_handler);
	float fltValue = 0;
	MmsValue *value;

	while (running) {
		Thread_sleep(1000);


		Timestamp iecTimestamp;
		uint64_t timestamp = Hal_getTimeInMs();


		Timestamp_clearFlags(&iecTimestamp);
		Timestamp_setTimeInMilliseconds(&iecTimestamp, timestamp);
		Timestamp_setLeapSecondKnown(&iecTimestamp, true);

		IedServer_updateTimestampAttributeValue(iedServer, IEDMODEL_PROT01_MMXU6_A_res_t, &iecTimestamp);


		value = MmsValue_newFloat(fltValue);
		IedServer_updateAttributeValue(iedServer, IEDMODEL_PROT01_MMXU6_A_res_cVal_mag_f, value);
		MmsValue_delete(value);

		fltValue += 5;

	}

	/* stop MMS server - close TCP server socket and all client sockets */
	IedServer_stop(iedServer);

	/* Cleanup - free all resources */
	IedServer_destroy(iedServer);
} /* main() */
