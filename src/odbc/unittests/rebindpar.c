#include "common.h"

/* Test for executing SQLExecute and rebinding parameters */

static char software_version[] = "$Id: rebindpar.c,v 1.8 2008-11-04 10:59:02 freddy77 Exp $";
static void *no_unused_var_warn[] = { software_version, no_unused_var_warn };

#define SWAP_STMT(b) do { SQLHSTMT xyz = Statement; Statement = b; b = xyz; } while(0)

static HSTMT stmt;

static void
TestInsert(char *buf)
{
	SQLLEN ind;
	int l = strlen(buf);
	char sql[200];

	/* insert some data and test success */
	CHKBindParameter(1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, l, 0, buf, l, &ind, "S");

	ind = l;
	CHKExecute("S");

	SWAP_STMT(stmt);
	sprintf(sql, "SELECT 1 FROM #tmp1 WHERE c = '%s'", buf);
	Command(Statement, sql);
	CHKFetch("S");
	CHKFetch("No");
	CHKMoreResults("No");
	SWAP_STMT(stmt);
}

static void
Test(int prebind)
{
	SQLLEN ind;
	int i;
	char buf[100];

	/* build a string longer than 80 character (80 it's the default) */
	buf[0] = 0;
	for (i = 0; i < 21; ++i)
		strcat(buf, "miao");

	Command(Statement, "DELETE FROM #tmp1");

	CHKAllocStmt(&stmt, "S");

	SWAP_STMT(stmt);
	if (prebind)
		CHKBindParameter(1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 1, 0, buf, 1, &ind, "S");

	CHKPrepare((SQLCHAR *) "INSERT INTO #tmp1(c) VALUES(?)", SQL_NTS, "S");

	/* try to insert an empty string, should not fail */
	/* NOTE this is currently the only test for insert a empty string using rpc */
	if (db_is_microsoft())
		TestInsert("");
	TestInsert("a");
	TestInsert("bb");
	TestInsert(buf);

	CHKFreeStmt(SQL_DROP, "S");
	Statement = SQL_NULL_HSTMT;
	SWAP_STMT(stmt);
}

int
main(int argc, char *argv[])
{
	Connect();

	Command(Statement, "CREATE TABLE #tmp1 (c VARCHAR(200))");

	Test(1);
	Test(0);

	Disconnect();

	printf("Done.\n");
	return 0;
}
