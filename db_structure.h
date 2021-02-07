struct customer{
	int account_number;
	char username[22];
	char password[17];
 	char type;
};

struct account{
	int account_number;
	double balance;
	char type;
};

struct transaction {
	int account_number;
	char username[22];
	char mode;
	double amount;
	int day;
	int month;
	int year;
	int hour;
	int min;
	int sec;
};

struct input {
	int user_choice;
	struct customer cus;
	struct account acc;
	struct transaction tran;
	int output;
};

struct login
{
	char username[22];
	char password[17];
};

struct transaction_entry {
	int flag;
	struct transaction tran;
};