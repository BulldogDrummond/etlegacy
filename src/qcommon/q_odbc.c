/*
 * ODBC Functions
 */

#ifdef FEATURE_ODBC

#include "q_odbc.h"

// Cvars
cvar_t *sv_odbcEnable;
cvar_t *sv_odbcDSN;
cvar_t *sv_odbcUsername;
cvar_t *sv_odbcPassword;

/* ODBC Error Handler */
void Com_ODBC_Error(
	char *fn,
	SQLHANDLE handle,
	SQLSMALLINT type)
{
	SQLINTEGER i = 0;
	SQLINTEGER native;
	SQLCHAR	state[ 7 ];
	SQLCHAR	text[256];
	SQLSMALLINT len;
	SQLRETURN ret;

	Com_Printf("ODBC Driver reported the following: %s\n",fn);
	do
	{
		ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len );
		if (SQL_SUCCEEDED(ret))
			Com_Printf("     %s:%ld:%ld:%s\n", state, i, native, text);
	}
	while( ret == SQL_SUCCESS );
}

/* This should be checked before any database operations are attempted. */
int Com_ODBC_Ready()
{
	int db_ready;

	cvar_t *dbTest = Cvar_Get("sv_dbReady", "0", CVAR_SERVERINFO | CVAR_ROM);
	db_ready = dbTest->integer;

	return db_ready;
}

/* TODO: This is basically just a function to test compile/link and basic functionality at the moment */
/*       A test like this needs to be done at InitGame to set the ready cvar, but the common portions
         should be separated into independant functions. */
void Com_ODBC_InitGameTest()
{
	sv_odbcEnable   = Cvar_Get("odbc_enable",   "0", CVAR_TEMP);
	sv_odbcDSN      = Cvar_Get("odbc_dsn",      "",  CVAR_TEMP);
	sv_odbcUsername = Cvar_Get("odbc_username", "",  CVAR_TEMP);
	sv_odbcPassword = Cvar_Get("odbc_password", "",  CVAR_TEMP);

	Cvar_Set("sv_odbcReady","0");

	int use_odbc;
	char *dsnname;
	char *username;
	char *password;

        use_odbc = sv_odbcEnable->integer;
	dsnname = sv_odbcDSN->string;
	username = sv_odbcUsername->string;
	password = sv_odbcPassword->string;

	if (use_odbc == 0) {
		Com_Printf("ODBC Info: ODBC Support Disabled.\n");
		Cvar_Set("sv_odbcReady","0");
		return;
	}

	Com_Printf("------ ODBC Initialization Test ------\n");

	SQLHENV env;
	SQLHDBC dbc;
	SQLHSTMT stmt;
	SQLRETURN ret;   /* ODBC API return status */
	SQLSMALLINT columns; /* number of columns in result-set */

	/* Allocate an environment handle */
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);

	/* We want ODBC 3 support */
	SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);

	/* Allocate a connection handle */
	Com_Printf("ODBC Info: Allocating connection handle to %s as user %s.\n",dsnname,username);
	SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

	/* Connect to the DSN */
	ret = SQLConnect(dbc, (SQLCHAR*) dsnname, SQL_NTS,
		(SQLCHAR*) username, SQL_NTS, (SQLCHAR*) password, SQL_NTS);	

	/* Allocate a statement handle */
	Com_Printf("ODBC Info: Allocating statement handle for testing.\n");
	SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);

	if (SQL_SUCCEEDED(ret)) {
		/* Report successful connection */
		Com_Printf("ODBC Info: Connected.\n");
		if (ret == SQL_SUCCESS_WITH_INFO) {
			Com_Printf("ODBC Driver reported the following:\n");
			Com_ODBC_Error("SQLConnect", dbc, SQL_HANDLE_DBC);
		}

		/* Set the cvar */
		Cvar_Set("sv_odbcReady","1");

		/* Display Driver Info */
		SQLCHAR dbms_name[256], dbms_ver[256];
		SQLUINTEGER getdata_support;
		SQLUSMALLINT max_concur_act;
		SQLSMALLINT string_len;

		SQLGetInfo(dbc, SQL_DBMS_NAME, (SQLPOINTER)dbms_name, sizeof(dbms_name), NULL);
		SQLGetInfo(dbc, SQL_DBMS_VER, (SQLPOINTER)dbms_ver, sizeof(dbms_ver), NULL);
		SQLGetInfo(dbc, SQL_GETDATA_EXTENSIONS, (SQLPOINTER)&getdata_support, 0, 0);
		SQLGetInfo(dbc, SQL_MAX_CONCURRENT_ACTIVITIES, &max_concur_act, 0, 0);

		Com_Printf("ODBC Info: Database Server: %s %s\n", dbms_name,dbms_ver);
		if (max_concur_act == 0) {
			Com_Printf("ODBC Info: Max Concurrent Activities: No limit or undefined.\n");
		} else {
			Com_Printf("ODBC Info: Max Concurrent Activities: %u.\n", max_concur_act);
		}
		if (getdata_support & SQL_GD_ANY_ORDER) {
			Com_Printf("ODBC Info: Columns can be retrieved in any order.\n");
		} else {
			Com_Printf("ODBC Info: Columns must be retrieved in order.\n");
		}
		if (getdata_support & SQL_GD_ANY_COLUMN) {
			Com_Printf("ODBC Info: Can retrieve columns before last bound.\n");
		} else {
			Com_Printf("ODBC Info: Columns must be retrieved after last bound.\n");
		}

		/* Run a test query */
		Com_Printf("ODBC Info: Running test query.\n");
		char *query = "SELECT ss_val FROM server_status WHERE ss_key = \"DB_Test\"";
		ret = SQLPrepare(stmt, (SQLCHAR *) query, strlen(query));
		ret = SQLExecute(stmt);
		if (SQL_SUCCEEDED(ret)) {
			SQLNumResultCols(stmt, &columns);
			while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
				SQLUSMALLINT i;
				for (i = 1; i <= columns; i++) {
					SQLINTEGER indicator;
					char buf[512];
					ret = SQLGetData(stmt, i, SQL_C_CHAR, buf, sizeof(buf), &indicator);
					if (SQL_SUCCEEDED(ret)) {
						if (indicator == SQL_NULL_DATA) strcpy(buf, "NULL");
						Com_Printf("ODBC Info: Query Result: %s\n",buf);
					}
				}
			}
		} else {
			Com_Printf("ODBC Info: Query failed.\n");
			Com_ODBC_Error("TestQuery",stmt,SQL_HANDLE_STMT);
		}
		SQLFreeHandle(SQL_HANDLE_STMT, stmt);
		SQLDisconnect(dbc);		/* disconnect from driver */
	} else {
		/* Report failed connection */
		Com_Printf("ODBC Info: Failed to connect.\n");
		Com_ODBC_Error("SQLDriverConnect", dbc, SQL_HANDLE_DBC);
		/* Set the cvar */
		Cvar_Set("sv_odbcReady","0");
	}

	/* free up allocated handles */
	SQLFreeHandle(SQL_HANDLE_DBC, dbc);
	SQLFreeHandle(SQL_HANDLE_ENV, env);
	Com_Printf("ODBC Info: Test Complete.\n");
	return;
}

#endif /* FEATURE_ODBC */

