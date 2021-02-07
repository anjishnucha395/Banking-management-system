
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db_structure.h"

int main()
{
	int fd1, fd2, fd3, fd4;
	struct customer cust;
	struct account acc;

	fd1=open("customer.db", O_CREAT|O_RDWR|O_TRUNC, 0644);

	if (fd1==-1)
	{
		perror("customer.db");
		exit(EXIT_FAILURE);
	}

	fd2=open("account.db", O_CREAT|O_RDWR|O_TRUNC, 0644);

	if (fd2==-1)
	{
		perror("account.db");
		exit(EXIT_FAILURE);
	}	

	fd3=open("account_sequence.db", O_CREAT|O_RDWR|O_TRUNC, 0644);

	if (fd3==-1)
	{
		perror("account.db");
		exit(EXIT_FAILURE);
	}

	fd4=open("transaction.db", O_CREAT|O_RDWR|O_TRUNC, 0644);

	if (fd4==-1)
	{
		perror("transaction.db");
		exit(EXIT_FAILURE);
	}
	// creating an admin to initiate banking process
	acc.account_number = 1;
	acc.balance = 0.00;
	acc.type = 'S';

	cust.account_number = 1;
	strcpy(cust.username, "BankAdmin");
	strcpy(cust.password, "admin1234");
	cust.type = 'A';

	if(write(fd2, &acc, sizeof(acc))==-1)
	{
		perror("Write error in account.db");
		exit(EXIT_FAILURE);
	}

	if(write(fd1, &cust, sizeof(cust))==-1)
	{
		perror("Write error in customer.db");
		exit(EXIT_FAILURE);
	}

	int last_acc_no = 1;
	
	if(write(fd3, &last_acc_no, sizeof(last_acc_no))==-1)
	{
		perror("Write error in account_sequence.db");
		exit(EXIT_FAILURE);
	}

	close(fd1);
	close(fd2);
	close(fd3);

	return 0;
}