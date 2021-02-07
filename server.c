#include <sys/types.h>          
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include "db_structure.h"

void check_credentials(char *, char *, struct customer *);
int does_username_exists(char *);
int does_account_exists(int );
void create_user_account(struct input *);
void change_password(struct input *);
void search_by_account(struct input *);
void search_by_username(struct input *);
void delete_by_username(struct input *);
void check_account_balance(struct input *);
void make_transaction(struct input *);
void display_transactions(struct input *, int);
bool acquire_lock(struct flock*, int , int , long int , long int , int , int);
void release_lock(struct flock*, int);

int main(int argc, char const *argv[])
{
	int sock_desc, size_client, client_sock_desc, pid;

	struct sockaddr_in server, client;

	sock_desc = socket(AF_INET, SOCK_STREAM, 0);

	if (sock_desc==-1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = INADDR_ANY; 
    server.sin_port = htons(8080);

	if (bind(sock_desc, (const struct sockaddr *)&server, sizeof(server))==-1)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(sock_desc, 128)==-1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	while(1) {

		size_client = sizeof(client);

		client_sock_desc = accept(sock_desc, (struct sockaddr *)&client, &size_client);

		if (client_sock_desc < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}
		pid = fork();

		if (pid==-1)
		{
			perror("fork");
			exit(EXIT_FAILURE);
		}
		else if(pid == 0) {
			close(sock_desc);
			// sending connection status

			char status = 'S';
			if (write(client_sock_desc, &status, sizeof(status))==-1) {
				perror("connection status sending");
				exit(EXIT_FAILURE);
			}

			struct login user;

			struct customer cust;

			while(1) {

				memset(&user, 0, sizeof(user));

				read(client_sock_desc, &user, sizeof(user));

				if (strlen(user.username) && strlen(user.password))
				{			
					check_credentials(user.username, user.password, &cust);

					if (write(client_sock_desc, &cust, sizeof(cust))==-1)
					{
						perror("\nUnable to send login status");
						exit(EXIT_FAILURE);
					}
					break;
				}
			}
			if (cust.type == 'A') 
			{
				printf("\nProcess %d corresponding to user %s started.", getpid(), cust.username);
				struct input inp[2];

				while(1) {
					inp[0].user_choice = -1;

					read(client_sock_desc, inp, sizeof(struct input)*2);

					switch(inp[0].user_choice) {

						case 1:
							create_user_account(inp);
							
							if(write(client_sock_desc, inp, sizeof(struct input)*2)==-1) {

								printf("\nSingle account creation status = %d for user %s", inp[0].output, inp[0].cus.username);
								perror("\nUnable to send single account creation status");
							}													
							break;
						case 2:
							create_user_account(inp);
						
							if(write(client_sock_desc, inp, sizeof(struct input)*2)==-1) {

								printf("\nJoint account creation status = %d for user %s and %s", inp[0].output, inp[0].cus.username, inp[1].cus.username);
								perror("\nUnable to send Joint account creation status");
							}													
							break;
						case 3:
							search_by_username(inp);

							if(write(client_sock_desc, inp, sizeof(struct input)*2)==-1) {

								printf("\nsearch by username status = %d for user %s", inp[0].output, inp[0].cus.username);
								perror("\nUnable to send username search status to client");
							}
							while(1) {
								inp[0].user_choice = 0;	
								read(client_sock_desc, inp, sizeof(struct input)*2);
								
								if (inp[0].user_choice == 1) 									
									break;

								else if(inp[0].user_choice == 3) {
									delete_by_username(inp);

									if(write(client_sock_desc, inp, sizeof(struct input)*2)==-1) {

										printf("\nDelete by username status = %d for user %s", inp[0].output, inp[0].cus.username);
										perror("\nUnable to send delete username status to client");
									}		
									break;
								}
							}
							break;
						case 4:
							change_password(inp);

							if(write(client_sock_desc, inp, sizeof(struct input)*2)==-1) {

								printf("\nPassword change status = %d for user %s", inp[0].output, inp[0].cus.username);
								perror("\nUnable to send password updation status");
							}												
							break;
						case 5:
							search_by_account(inp);

							if(write(client_sock_desc, inp, sizeof(struct input)*2)==-1) {

								printf("\nAccount search status = %d", inp[0].output);
								perror("\nUnable to send account search status to client");
							}					
							break;
						case 6:		
							close(client_sock_desc);
							printf("\nProcess %d corresponding to user %s terminated.", getpid(), cust.username);
							exit(EXIT_SUCCESS);						
					}
				}
			}
			else if(cust.type == 'U') {
				printf("\nProcess %d corresponding to user %s started.", getpid(), cust.username);
				struct input inp[2];

				while(1) {
					inp[0].user_choice = -1;

					read(client_sock_desc, inp, sizeof(struct input)*2);

					switch(inp[0].user_choice) {

						case 1:	
							make_transaction(inp);

							if(write(client_sock_desc, inp, sizeof(struct input)*2)==-1) {

								printf("\nDeposit status = %d", inp[0].output);
								perror("\nUnable to send balance data to client");
							}																		
							break;
						case 2:	
							make_transaction(inp);

							if(write(client_sock_desc, inp, sizeof(struct input)*2)==-1) {

								printf("\nWithdraw status = %d", inp[0].output);
								perror("\nUnable to send balance data to client");
							}												
							break;
						case 3:
							check_account_balance(inp);

							if(write(client_sock_desc, inp, sizeof(struct input)*2)==-1) {

								printf("\nBalance display status = %d", inp[0].output);
								perror("\nUnable to send balance data to client");
							}
							break;
						case 4:	
							change_password(inp);

							if(write(client_sock_desc, inp, sizeof(struct input)*2)==-1) {

								printf("\nPassword change status = %d for user %s", inp[0].output, inp[0].cus.username);
								perror("\nUnable to password updation status");
							}																							
							break;
						case 5:
							search_by_account(inp);

							if(write(client_sock_desc, inp, sizeof(struct input)*2)==-1) {

								printf("\nUser view status = %d", inp[0].output);
								perror("\nUnable to send user account information to client");
							}	

							display_transactions(inp, client_sock_desc);				
							break;
						case 6:	
							close(client_sock_desc);
							printf("\nProcess %d corresponding to user %s terminated.", getpid(), cust.username);
							exit(EXIT_SUCCESS);						
					}
				}
			}
			else {
				close(client_sock_desc);
				exit(EXIT_SUCCESS);				
			}
		}
		else
			close(client_sock_desc);
	}

	return 0;
}

void check_credentials(char *usr, char *pwd, struct customer *cust) {

	int fd;
	struct flock lock;
	fd=open("customer.db",O_RDONLY);

	if(fd==-1) {
		perror("\nunable to access customer database");
		cust->type = 'F';
		return;
	}
	if (!acquire_lock(&lock, F_RDLCK, SEEK_SET, 0 ,0, getpid(), fd)) {
		cust->type = 'F';
		close(fd);
		return;		
	}
	ssize_t data = 0;

	while(1) {
		data = read(fd, cust, sizeof(struct customer));
	
		if (data == -1) {
			perror("Accessing customer.db during login");
			cust->type = 'F';
			release_lock(&lock, fd);
			close(fd);
			return;
		}
		else if(data == 0) {
			cust->type = 'N';
			release_lock(&lock, fd);
			close(fd);
			return;
		}
		else if(strcmp(cust->username, usr)==0 && strcmp(cust->password, pwd)==0) {
			release_lock(&lock, fd);		
			close(fd);	
			break;
		}				
	}
}

int does_username_exists(char *usr) {

	int fd;
	struct customer cust;
	struct flock lock;
	
	fd=open("customer.db",O_RDONLY);

	if(fd==-1) {
		perror("unable to access customer database");
		return 1;		
	}
	if (!acquire_lock(&lock, F_RDLCK, SEEK_SET, 0 ,0, getpid(), fd))
	{
		perror("Accessing customer.db");
		close(fd);
		return 1;
	}
	ssize_t data;

	while(1) {

		data = read(fd, &cust, sizeof(struct customer));
	
		if (data == -1) {
			perror("Accessing customer.db");
			release_lock(&lock, fd);
			close(fd);
			return 1;
		}
		else if(data == 0) {
			release_lock(&lock, fd);
			close(fd);
			return 2;
		}

		else if(strcmp(cust.username, usr)==0) {
			release_lock(&lock, fd);
			close(fd);
			return 3;
		}				
	}
}

int does_account_exists(int acno) {

	int fd;
	struct account acc;
	struct flock lock;
	
	fd=open("account.db",O_RDONLY);

	if(fd==-1) {
		perror("unable to access customer database");
		return 1;
	}
	if (!acquire_lock(&lock, F_RDLCK, SEEK_SET, 0 ,0, getpid(), fd)) {
		perror("Accessing account.db");
		close(fd);
		return 1;
	}
	ssize_t data;

	while(1) {

		data = read(fd, &acc, sizeof(struct account));
	
		if (data == -1)
		{
			perror("Accessing account.db");
			release_lock(&lock, fd);
			close(fd);
			return 1;
		}
		else if(data == 0) {
			release_lock(&lock, fd);
			close(fd);
			return 2;
		}
		else if(acc.account_number == acno) {
			release_lock(&lock, fd);
			close(fd);
			return 3;
		}
	}
}

void create_user_account(struct input *inp) {

	int account_number, fd1, fd2, fd3;
	struct flock lock;
	ssize_t size;
	struct account *acc;
	struct customer *cus;

	int result = does_username_exists(inp[0].cus.username);

	if(result == 3) {
		inp[0].output = 1;
		return;
	}
	else if(result == 1) {
		inp[0].output = 3;
		return;
	}
	switch(inp[0].user_choice) {
		case 2:
			result = does_username_exists(inp[1].cus.username);

			if(result == 3) {
				inp[0].output = 2;
				return;
			}
			else if(result == 1) {
				inp[0].output = 3;
				return;
			}
	}
	fd1=open("account_sequence.db",O_RDWR);

	if(fd1==-1) {
		perror("\nunable to access account_sequence.db");
		inp[0].output = 3;
		return;
	}			
	// Take write lock on account_sequence.db 
	if (!acquire_lock(&lock, F_WRLCK, SEEK_SET, 0 ,0, getpid(), fd1)) {
		inp[0].output = 3;
		close(fd1);
		return;		
	}
	size = read(fd1, &account_number, sizeof(account_number));
			
	if (size == -1) {
		perror("\nAccessing account_sequence.db");
		inp[0].output = 3;
		release_lock(&lock, fd1);
		close(fd1);
		return;
	}
	else if(size == 0) {
		printf("Nothing is read in account_sequence.db\n");
		inp[0].output = 3;
		release_lock(&lock, fd1);
		close(fd1);
		return;
	}
	else if(account_number<=0) {
		inp[0].output = 3;
		release_lock(&lock, fd1);
		close(fd1);
		return;
	}
	else
		account_number++;

	lseek(fd1, 0, SEEK_SET);

	if (write(fd1, &account_number, sizeof(account_number))==-1) {

		perror("Account_sequence.db cannot be modified");
		inp[0].output = 3;
		release_lock(&lock, fd1);
		close(fd1);
		return;
	}

	fd2=open("customer.db",O_RDWR);

	if(fd2==-1) {
		perror("\nunable to access customer.db");
		inp[0].output = 3;
		release_lock(&lock, fd1);
		close(fd1);
		return;
	}
	// Take write lock on customer.db at the end 
	if (!acquire_lock(&lock, F_WRLCK, SEEK_END, 0, sizeof(struct customer)*2, getpid(), fd2)) {
		inp[0].output = 3;
		release_lock(&lock, fd1);
		close(fd1);
		close(fd2);
		return;		
	}

	switch(inp[0].user_choice) {

		case 1:
			// create record in customer.db
			cus = (struct customer *)malloc(sizeof(struct customer));
			cus->account_number = account_number;
			strcpy(cus->username, inp[0].cus.username);
			strcpy(cus->password, inp[0].cus.password);
			cus->type = 'U';

			lseek(fd2, 0, SEEK_END);

			if (write(fd2, cus, sizeof(struct customer))==-1) {
				// Apply rollback mechanism on sequence
				perror("Record cannot be added to customer.db");
				inp[0].output = 3;
				release_lock(&lock, fd1);
				release_lock(&lock, fd2);
				close(fd1);
				close(fd2);
				return;
			}	

			acc = (struct account *)malloc(sizeof(struct account));
			acc->type = 'S';	
			break;
		case 2:
			// create record in customer.db
			cus = (struct customer *)malloc(sizeof(struct customer)*2);

			for(int i=0; i<2; i++) {
				cus[i].account_number = account_number;
				strcpy(cus[i].username, inp[i].cus.username);
				strcpy(cus[i].password, inp[i].cus.password);
				cus[i].type = 'U';				
			}

			lseek(fd2, 0, SEEK_END);

			if (write(fd2, &cus[0], sizeof(struct customer))==-1) {
				// Apply rollback mechanism on sequence
				perror("Record cannot be added to customer.db");
				inp[0].output = 3;
				release_lock(&lock, fd1);
				release_lock(&lock, fd2);
				close(fd1);
				close(fd2);
				return;
			}	

			if (write(fd2, &cus[1], sizeof(struct customer))==-1) {
				// Apply rollback mechanism on sequence
				perror("Record cannot be added to customer.db");
				inp[0].output = 3;
				release_lock(&lock, fd1);
				release_lock(&lock, fd2);
				close(fd1);
				close(fd2);
				return;
			}
		
			acc = (struct account *)malloc(sizeof(struct account));
			acc->type = 'J';
	}
	acc->account_number = account_number;
	acc->balance = 0.00;

	fd3=open("account.db",O_RDWR);

	if(fd3==-1) {
		perror("\nunable to access account.db");
		inp[0].output = 3;
		release_lock(&lock, fd1);
		release_lock(&lock, fd2);
		close(fd1);
		close(fd2);
		return;
	}
	// Take write lock on account.db at the end 
	if (!acquire_lock(&lock, F_WRLCK, SEEK_END, 0, sizeof(struct account), getpid(), fd3)) {
		inp[0].output = 3;
		release_lock(&lock, fd1);
		release_lock(&lock, fd2);
		close(fd1);
		close(fd2);		
		close(fd3);
		return;		
	}

	lseek(fd3, 0, SEEK_END);
	// create record in account.db
	if (write(fd3, acc, sizeof(struct account))==-1) {
		// Apply rollback mechanism on sequence and customer
		perror("Record cannot be added to account.db");
		inp[0].output = 3;
		release_lock(&lock, fd1);
		release_lock(&lock, fd2);
		release_lock(&lock, fd3);
		close(fd1);
		close(fd2);
		close(fd3);
		return;
	}

	// Release all the lock
	release_lock(&lock, fd1);
	release_lock(&lock, fd2);
	release_lock(&lock, fd3);
	close(fd1);
	close(fd2);
	close(fd3);

	inp[0].output = 4;
	inp[0].cus.account_number = account_number;
}

void change_password(struct input *inp) {

	int fd;
	ssize_t size;
	struct customer cust;
	struct flock lock;
	fd=open("customer.db",O_RDWR);

	if(fd==-1) {
		perror("\nunable to access customer.db");
		inp[0].output = 3;
		return;
	}	

	if (!acquire_lock(&lock, F_RDLCK, SEEK_SET, 0 ,0, getpid(), fd)) {
		inp[0].output = 3;
		close(fd);
		return;
	}

	while(1) {

		size = read(fd, &cust, sizeof(cust));

		if(size == 0) {
			release_lock(&lock, fd);
			close(fd);
			inp[0].output = 1;
			return;
		}
		else if(size == -1) {
			release_lock(&lock, fd);
			close(fd);
			perror("\nRead error in customer.db");
			inp[0].output = 3;
			return;
		}
		else if(strcmp(cust.username, inp[0].cus.username)==0)
			break;
	}
	if (strcmp(cust.password, inp[0].cus.password)!=0) {
		release_lock(&lock, fd);
		close(fd);
		inp[0].output = 2;
		return;
	}

	lseek(fd, -1*sizeof(struct customer), SEEK_CUR);

	release_lock(&lock, fd);

	if (!acquire_lock(&lock, F_WRLCK, SEEK_CUR, 0 ,sizeof(struct customer), getpid(), fd)) {
		inp[0].output = 3;
		close(fd);
		return;
	}

	strcpy(cust.password, inp[1].cus.password);

	if (write(fd, &cust, sizeof(struct customer))==-1) {
		release_lock(&lock, fd);
		close(fd);
		perror("write error in customer.db");
		inp[0].output = 3;
		return;
	}
	release_lock(&lock, fd);
	close(fd);

	inp[0].output = 4;
}

void search_by_account(struct input *inp) {

	int fd1, fd2,i=-1;
	ssize_t size;
	struct customer cust;
	struct account acc;
	struct flock lock;
	int result = does_account_exists(inp[0].cus.account_number);

	if(result == 1) {
		inp[0].output = 2;
		return;
	}
	else if(result == 2) {
		inp[0].output = 1;
		return;
	}
	fd2=open("account.db",O_RDONLY);

	if(fd2==-1) {
		perror("\nunable to access account.db");
		inp[0].output=2;
		return;
	}
	fd1=open("customer.db",O_RDONLY);

	if(fd1==-1) {
		perror("\nunable to access account.db");
		inp[0].output=2;
		close(fd2);
		return;
	}
	if (!acquire_lock(&lock, F_RDLCK, SEEK_SET, (inp[0].cus.account_number-1) *sizeof(acc),sizeof(acc), getpid(), fd2)) {
		inp[0].output=2;
		close(fd2);
		close(fd1);
		return;
	}

	if (!acquire_lock(&lock, F_RDLCK, SEEK_SET, 0 ,0, getpid(), fd1)) {
		inp[0].output=2;
		release_lock(&lock, fd2);
		close(fd2);
		close(fd1);
		return;
	}

	lseek(fd2, (inp[0].cus.account_number-1) *sizeof(acc), SEEK_SET);

	size = read(fd2, &acc, sizeof(acc));

	if(size == 0 || acc.account_number == -1) {
		release_lock(&lock, fd2);
		release_lock(&lock, fd1);
		close(fd2);
		close(fd1);
		inp[0].output=1;
		printf("Account is deleted after successfull search initially\n");
		return;
	}
	else if(size == -1) {
		release_lock(&lock, fd2);
		release_lock(&lock, fd1);
		close(fd2);
		close(fd1);
		inp[0].output=2;
		perror("\nread error in account.db");
		return;
	}

	inp[0].acc.balance = acc.balance;
	inp[0].acc.type = acc.type;	

	while(1) {

		size = read(fd1, &cust, sizeof(cust));

		if(size == -1) {
			release_lock(&lock, fd2);
			release_lock(&lock, fd1);
			close(fd2);
			close(fd1);
			inp[0].output=2;
			return;
		}
		else if(size == 0) {
			release_lock(&lock, fd2);
			release_lock(&lock, fd1);
			close(fd2);
			close(fd1);
			printf("Every customer is not fetched\n");
			inp[0].output=2;
			return;
		}
		else if(cust.account_number == inp[0].cus.account_number) {
			i++;
			strcpy(inp[i].cus.username, cust.username);
			inp[i].cus.type = cust.type;
			if(acc.type == 'S' || i == 1)
				break;
		}
	}
	release_lock(&lock, fd2);
	release_lock(&lock, fd1);
	close(fd2);
	close(fd1);

	inp[0].output=3;	
}

void search_by_username(struct input *inp) {

	int fd1, fd2;
	ssize_t size;
	struct customer cust;
	struct account acc;
	struct flock lock;
	fd1=open("customer.db",O_RDONLY);

	if(fd1==-1) {
		perror("\nunable to access customer.db");
		inp[0].output = 3;
		return;
	}	
	if (!acquire_lock(&lock, F_RDLCK, SEEK_SET, 0 ,0, getpid(), fd1)) {
		inp[0].output=3;
		close(fd1);
		return;
	}
	while(1) {
		size = read(fd1, &cust, sizeof(cust));

		if(size == 0) {
			release_lock(&lock, fd1);
			close(fd1);
			inp[0].output = 1;
			return;
		}
		else if(size == -1) {
			release_lock(&lock, fd1);
			close(fd1);
			perror("\nRead error in customer.db");
			inp[0].output = 3;
			return;
		}
		else if(strcmp(cust.username, inp[0].cus.username)==0)
			break;
	}
	if (strcmp(cust.password, inp[0].cus.password)!=0) {
		release_lock(&lock, fd1);
		close(fd1);
		inp[0].output = 2;
		return;
	}

	inp[0].acc.account_number = cust.account_number;
	inp[0].cus.type = cust.type;

	fd2=open("account.db",O_RDONLY);

	if(fd2==-1) {
		perror("\nunable to access account.db");
		release_lock(&lock, fd1);
		close(fd1);
		inp[0].output = 3;
		return;
	}	
	if (!acquire_lock(&lock, F_RDLCK, SEEK_SET, (inp[0].acc.account_number-1) *sizeof(acc),sizeof(acc), getpid(), fd2)) {
		release_lock(&lock, fd1);
		close(fd1);
		close(fd2);
		inp[0].output=3;
		return;
	}
	lseek(fd2, (inp[0].acc.account_number-1) *sizeof(acc), SEEK_SET);

	size = read(fd2, &acc, sizeof(acc));

	if(size == -1) {
		release_lock(&lock, fd1);
		release_lock(&lock, fd2);
		close(fd1);
		close(fd2);
		inp[0].output=3;
		perror("\nread error in account.db");
		return;
	}
	else if(size == 0 || acc.account_number == -1 || acc.account_number != inp[0].acc.account_number) {
		release_lock(&lock, fd1);
		release_lock(&lock, fd2);
		close(fd1);
		close(fd2);
		inp[0].output=1;
		printf("Account for customer %s is already deleted\n",inp[0].cus.username);
		return;
	}
	release_lock(&lock, fd1);
	release_lock(&lock, fd2);
	close(fd1);
	close(fd2);

	inp[0].acc.balance = acc.balance;
	inp[0].acc.type = acc.type;		

	inp[0].output = 4;
}

void delete_by_username(struct input *inp) {

	int fd1, fd2;
	ssize_t size;
	struct customer cust;
	struct account acc;
	struct flock lock;
	fd1=open("customer.db",O_RDWR);

	if(fd1==-1) {
		perror("\nunable to access customer.db");
		inp[0].output = 2;
		return;
	}
	fd2=open("account.db",O_RDWR);

	if(fd2==-1) {
		perror("\nunable to access account.db");
		close(fd1);
		inp[0].output = 2;
		return;
	}	
	if (!acquire_lock(&lock, F_WRLCK, SEEK_SET, (inp[0].acc.account_number-1)*sizeof(acc) ,sizeof(acc), getpid(), fd2)) {
		inp[0].output=2;
		close(fd1);
		close(fd2);
		return;
	}
	while(1) {
		size = read(fd1, &cust, sizeof(cust));

		if(size == 0) {
			inp[0].output = 1;
			release_lock(&lock, fd2);
			close(fd2);
			close(fd1);
			return;
		}
		else if(size == -1) {
			perror("\nRead error in customer.db");
			inp[0].output = 3;
			release_lock(&lock, fd2);
			close(fd2);
			close(fd1);
			return;
		}
		else if(strcmp(cust.username, inp[0].cus.username)==0)
			break;
	}
	if (!acquire_lock(&lock, F_WRLCK, SEEK_CUR, -1*sizeof(cust) ,sizeof(cust), getpid(), fd1)) {
		inp[0].output=3;
		release_lock(&lock, fd2);
		close(fd2);
		close(fd1);		
		return;
	}	
	lseek(fd1, -1*sizeof(cust), SEEK_CUR);

	cust.account_number = -1;
	cust.username[0]='\0';
	cust.password[0]='\0';
	cust.type = '\0';

	if (write(fd1, &cust, sizeof(cust))==-1) {
		release_lock(&lock, fd2);
		release_lock(&lock, fd1);
		close(fd2);
		close(fd1);
		inp[0].output=2;
		return;
	}

	lseek(fd2, (inp[0].acc.account_number-1) *sizeof(acc), SEEK_SET);

	if (inp[0].acc.type == 'S') {
		acc.account_number = -1;
		acc.balance = 0.00;
		acc.type = '\0';
	}
	else if(inp[0].acc.type == 'J') {
		size = read(fd2, &acc, sizeof(acc));

		if(size == -1) {
			release_lock(&lock, fd2);
			release_lock(&lock, fd1);
			close(fd2);
			close(fd1);
			inp[0].output=2;
			perror("\nread error in account.db");
			return;
		}
		else if(size == 0 || acc.account_number == -1 || inp[0].acc.account_number != acc.account_number) {
			release_lock(&lock, fd2);
			release_lock(&lock, fd1);
			close(fd2);
			close(fd1);
			inp[0].output=1;
			printf("Account is found to be alreay deleted when another deletion take place\n");
			return;
		}	
		acc.type = 'S';
	}

	lseek(fd2, (inp[0].acc.account_number-1) *sizeof(acc), SEEK_SET);

	if (write(fd2, &acc, sizeof(acc))==-1) {
		release_lock(&lock, fd2);
		release_lock(&lock, fd1);
		close(fd2);
		close(fd1);
		inp[0].output=2;
		return;
	}
	
	release_lock(&lock, fd2);
	release_lock(&lock, fd1);
	close(fd2);
	close(fd1);

	inp[0].output = 3;
}

void check_account_balance(struct input *inp) {

	int fd2;
	ssize_t size;
	struct customer cust;
	struct account acc;
	struct flock lock;

	fd2=open("account.db",O_RDONLY);

	if(fd2==-1) {
		perror("\nunable to access account.db");
		inp[0].output=2;
		return;
	}
	if (!acquire_lock(&lock, F_RDLCK, SEEK_SET, (inp[0].acc.account_number-1) *sizeof(acc),sizeof(acc), getpid(), fd2)) {
		inp[0].output=2;
		close(fd2);
		return;
	}

	lseek(fd2, (inp[0].acc.account_number-1) *sizeof(acc), SEEK_SET);

	size = read(fd2, &acc, sizeof(acc));

	if(acc.account_number == -1) {
		release_lock(&lock, fd2);
		close(fd2);
		inp[0].output=1;
		printf("Account information of customer %s is deleted\n",inp[0].cus.username);
		return;
	}
	else if(size == -1) {
		release_lock(&lock, fd2);
		close(fd2);
		inp[0].output=2;
		perror("\nread error in account.db");
		return;
	}

	inp[0].acc.balance = acc.balance;

	release_lock(&lock, fd2);
	close(fd2);

	inp[0].output=3;		
}
void make_transaction(struct input *inp) {

	int fd2, fd3;
	ssize_t size;
	struct customer cust;
	struct account acc;
	struct transaction tran;
	struct flock lock;

	tran.account_number = inp[0].acc.account_number;
	strcpy(tran.username, inp[0].cus.username);

	fd2=open("account.db",O_RDWR);

	if(fd2==-1) {
		perror("\nunable to access account.db");
		inp[0].output=2;
		return;
	}
	if (!acquire_lock(&lock, F_WRLCK, SEEK_SET, (inp[0].acc.account_number-1) *sizeof(acc),sizeof(acc), getpid(), fd2)) {
		inp[0].output=2;
		close(fd2);
		return;
	}
	lseek(fd2, (inp[0].acc.account_number-1) *sizeof(acc), SEEK_SET);

	size = read(fd2, &acc, sizeof(acc));

	if(size == 0 || acc.account_number == -1) {
		release_lock(&lock, fd2);
		close(fd2);
		inp[0].output=1;
		printf("Account information is deleted for user %s\n",inp[0].cus.username);
		return;
	}
	else if(size == -1) {
		release_lock(&lock, fd2);
		close(fd2);
		inp[0].output=2;
		perror("\nread error in account.db");
		return;
	}
	else if(inp[0].acc.account_number != acc.account_number) {
		release_lock(&lock, fd2);
		close(fd2);
		inp[0].output=1;
		return;		
	}
	tran.amount = inp[0].acc.balance;

	switch(inp[0].user_choice) {
		case 1:
			acc.balance += inp[0].acc.balance;				
			tran.mode = 'C';	
			break;
		case 2:
			if(inp[0].acc.balance > acc.balance) {
				inp[0].output = 4;
				return;
			}
			acc.balance -= inp[0].acc.balance;	
			tran.mode = 'D';				
	}

	lseek(fd2, -1*sizeof(acc), SEEK_CUR);

	if (write(fd2, &acc, sizeof(acc))==-1) {
		release_lock(&lock, fd2);
		close(fd2);
		perror("write error in account.db");
		inp[0].output = 2;
		return;
	}	
	release_lock(&lock, fd2);
	close(fd2);

	inp[0].acc.balance = acc.balance;

	time_t t = time(NULL);
	struct tm date_time = *localtime(&t);

	inp[0].output = 3;

	fd3=open("transaction.db",O_RDWR);

	if(fd3==-1) {
		perror("\nunable to access transaction.db");
		printf("transaction is not added for user %s. Process %d\n", inp[0].cus.username,getpid());
		return;
	}

	if (!acquire_lock(&lock, F_WRLCK, SEEK_END, 0, sizeof(tran), getpid(), fd3)) {
		printf("Write lock cannot be acquired in transaction.db for %s. Process %d\n", inp[0].cus.username, getpid());	
		close(fd3);
		return;
	}

	lseek(fd3, 0, SEEK_END);

	tran.day = date_time.tm_mday;
	tran.month = date_time.tm_mon + 1;
	tran.year = date_time.tm_year + 1900;

	tran.hour = date_time.tm_hour;
	tran.min = date_time.tm_min;
	tran.sec = date_time.tm_sec;

	if (write(fd3, &tran, sizeof(tran))==-1) {
		release_lock(&lock, fd3);
		close(fd3);
		perror("write error in transaction.db for %s\n");
		printf("username: %s, process = %d\n", inp[0].cus.username, getpid());
		return;
	}

	release_lock(&lock, fd3);
	close(fd3);
}

void display_transactions(struct input *inp, int client_sock_desc) {

	int fd;
	struct transaction t;
	struct transaction_entry t_entry;
	bool next;
	ssize_t size;

	fd=open("transaction.db",O_RDONLY);

	if(fd==-1) {
		perror("\nunable to access transaction.db");
		return;
	}
	while(1) {
		size = read(fd, &t, sizeof(t));

		if(size == 0) {
			close(fd);
			t_entry.flag=2;

			if(write(client_sock_desc, &t_entry, sizeof(t_entry))==-1) {
				perror("\nUnable to send transaction information to client");
			}						
			return;
		}
		else if(size == -1) {
			close(fd);
			perror("\nread error in transaction.db");
			t_entry.flag = 1;

			if(write(client_sock_desc, &t_entry, sizeof(t_entry))==-1) {
				perror("\nUnable to send transaction information to client");
			}			
			return;
		}
		else if(t.account_number == inp[0].cus.account_number) {
			t_entry.flag=3;

			t_entry.tran.mode = t.mode;
			t_entry.tran.amount = t.amount;

			t_entry.tran.day = t.day;
			t_entry.tran.month = t.month;
			t_entry.tran.year = t.year;

			t_entry.tran.hour = t.hour;
			t_entry.tran.min = t.min;
			t_entry.tran.sec = t.sec;

			if(write(client_sock_desc, &t_entry, sizeof(t_entry))==-1) {
				perror("\nUnable to send transaction information to client");
				close(fd);
				return;
			}	
			while(1) {
				next = false;
				read(client_sock_desc, &next, sizeof(bool));
				if (next) 
					break;
			}
		}
	}
}
bool acquire_lock(struct flock* lock_address, int type, int whence, long int start, long int len, int pid , int fd) {

	lock_address->l_type=type;
	lock_address->l_whence=whence;
	lock_address->l_start=start;
	lock_address->l_len=len;
	lock_address->l_pid=pid;

	if (fcntl(fd, F_SETLKW, lock_address)==-1)
		return false;

	return true;
}

void release_lock(struct flock* lock_address, int fd) {

	lock_address->l_type=F_UNLCK;

	if (fcntl(fd, F_SETLK, lock_address)==-1)
		perror("Cannot release lock");		
}
