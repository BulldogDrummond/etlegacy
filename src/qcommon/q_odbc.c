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
			Com_Printf("  %s:%ld:%ld:%s\n", state, i, native, text);
	}
	while( ret == SQL_SUCCESS );
}

/* TODO: This is basically just a function to test compile/link at the moment */
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
		Com_Printf("ODBC Support Disabled.\n");
		Cvar_Set("sv_odbcReady","0");
		return;
	}

	Com_Printf("------ ODBC Initialization Test ------\n");

	SQLHENV env;
	SQLHDBC dbc;
	SQLHSTMT stmt;
	SQLRETURN ret; /* ODBC API return status */
	SQLCHAR outstr[1024];
	SQLSMALLINT outstrlen;

	/* Allocate an environment handle */
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);

	/* We want ODBC 3 support */
	SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);

	/* Allocate a connection handle */
	Com_Printf("ODBC Allocating connection handle to %s as user %s.\n",dsnname,username);
	SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);

	/* Connect to the DSN */
	ret = SQLConnect(dbc, (SQLCHAR*) dsnname, SQL_NTS,
		(SQLCHAR*) username, SQL_NTS, (SQLCHAR*) password, SQL_NTS);	

	if (SQL_SUCCEEDED(ret)) {
		/* Report successful connection */
		Com_Printf("ODBC Connected.\n");
		if (ret == SQL_SUCCESS_WITH_INFO) {
			Com_Printf("ODBC Driver reported the following:\n");
			Com_ODBC_Error("SQLDriverConnect", dbc, SQL_HANDLE_DBC);
		}
		/* Set the cvar */
		Cvar_Set("sv_odbcReady","1");
		SQLDisconnect(dbc);		/* disconnect from driver */
	} else {
		/* Report failed connection */
		Com_Printf("ODBC Failed to connect\n");
		Com_ODBC_Error("SQLDriverConnect", dbc, SQL_HANDLE_DBC);
		/* Set the cvar */
		Cvar_Set("sv_odbcReady","0");
	}
	/* free up allocated handles */
	SQLFreeHandle(SQL_HANDLE_DBC, dbc);
	SQLFreeHandle(SQL_HANDLE_ENV, env);
	Com_Printf("ODBC Test Complete.\n");
	return;
}

#endif /* FEATURE_ODBC */

