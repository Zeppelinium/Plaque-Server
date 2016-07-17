#include <c.h>
#include <string.h>

#include "db.h"
#include "mmps.h"
#include "paquet.h"
#include "report.h"
#include "reports.h"
#include "tasks.h"

int
reportMessage(struct paquet *paquet)
{
	struct task	*task = paquet->task;

	const char	*paramValues[2];
    Oid			paramTypes[2];
    int			paramLengths[2];
	int			paramFormats[2];

	struct MMPS_Buffer *inputBuffer = paquet->inputBuffer;
	struct MMPS_Buffer *outputBuffer = paquet->inputBuffer;

	if (!minimumPayloadSize(paquet, sizeof(struct paquetReport))) {
		setTaskStatus(task, TaskStatusWrongPayloadSize);
		return -1;
	}

	MMPS_ResetCursor(inputBuffer, 1);

	struct paquetReport payload;

	inputBuffer = MMPS_GetData(inputBuffer, (char *)&payload, sizeof(payload));

	int expectedSize = sizeof(payload) + be32toh(payload.messageLength);
	if (!expectedPayloadSize(paquet, expectedSize)) {
		setTaskStatus(task, TaskStatusWrongPayloadSize);
		return -1;
	}

	char *message = (char *)malloc(payload.messageLength);
	if (message == NULL) {
		reportError("Cannot allocate %ud bytes for report message",
			payload.messageLength);
		setTaskStatus(task, TaskStatusOutOfMemory);
		return -1;
	}

	inputBuffer = MMPS_GetData(inputBuffer, message, payload.messageLength);

	struct dbh *dbh = DB_PeekHandle(task->desk->db.plaque);
	if (dbh == NULL) {
		free(message);
		setTaskStatus(task, TaskStatusNoDatabaseHandlers);
		return -1;
	}

	paramValues   [0] = (char *)&task->deviceId;
	paramTypes    [0] = INT8OID;
	paramLengths  [0] = sizeof(uint64);
	paramFormats  [0] = 1;

	paramValues   [1] = (char *)message;
	paramTypes    [1] = TEXTOID;
	paramLengths  [1] = payload.messageLength;
	paramFormats  [1] = 0;

	dbh->result = PQexecParams(dbh->conn, "\
INSERT INTO debug.reports (device_id, message) \
VALUES ($1, $2)",
		2, paramTypes, paramValues, paramLengths, paramFormats, 1);

	if (!DB_CommandOK(dbh, dbh->result)) {
		DB_PokeHandle(dbh);
		free(message);
		setTaskStatus(task, TaskStatusUnexpectedDatabaseResult);
		return -1;
	}

	MMPS_ResetBufferData(outputBuffer, 1);

	DB_PokeHandle(dbh);

	free(message);

	paquet->outputBuffer = paquet->inputBuffer;

	return 0;
}
