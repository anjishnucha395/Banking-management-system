
#include <sys/types.h>          
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "db_structure.h"

void showLoginPrompt(int, struct customer *);
int showAdminOption(char *);
int showCustomerOption(char *);

int main(int argc, char const *argv[])
{
	int sock_desc;
	struct sockaddr_in server;
	ssize_t data_size;
	struct customer cust;

	sock_desc = socket(AF_INET, SOCK_STREAM, 0);

	if (sock_desc==-1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = INADDR_ANY; 
    server.sin_port = htons(8080);

	if (connect(sock_desc, (const struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("connect");
		exit(EXIT_FAILURE);
	}

	char status='\0';

	while(1) {
		data_size = read(sock_desc, &status, sizeof(status));

		if (data_size==-1) {
			perror("No communication from server");
			exit(EXIT_FAILURE);
		}
		else if(status == 'S')
			break;
	}

	showLoginPrompt(sock_desc, &cust);

	if (cust.type == 'F') {
		printf("\nLogin unsuccessful due to server issue\n");
		exit(EXIT_SUCCESS);
	}
	else if (cust.type == 'N') {
		printf("\nusername or password does not match!. Please try again.\n");
		exit(EXIT_SUCCESS);
	}
	else if(cust.type == 'A') {
		printf("\n***********************Welcome %s***********************\n",cust.username);

		struct input inp[2];

		while(1) {

			int choice = showAdminOption(cust.username);
			ssize_t size;
			
			switch(choice) {

				case 1:
					printf("\n------------Add a single account------------\n");

					printf("\nEnter username\n");
					size = read(0, &(inp[0].cus.username), sizeof(inp[0].cus.username));				
					inp[0].cus.username[size-1]='\0';

					printf("Enter password\n");
					size = read(0, &(inp[0].cus.password), sizeof(inp[0].cus.password));				
					inp[0].cus.password[size-1]='\0';	

					if(strlen(inp[0].cus.username)<1) {
						printf("\nusername cannot be empty.\n");
						break;
					}
					if(strlen(inp[0].cus.password)<1) {
						printf("\npassword cannot be empty\n");
						break;
					}
					if(strlen(inp[0].cus.username)>20) {
						printf("\nusername length exceeds 20!.\n");
						break;
					}
					if(strlen(inp[0].cus.password)>15) {
						printf("\npassword length exceeds 15!\n");
						break;
					}

					inp[0].user_choice = 1;

					if (write(sock_desc, inp, sizeof(struct input)*2)==-1)
					{
						perror("\nUanble to send data to server");
						exit(EXIT_FAILURE);
					}
					
					while(1) {
						inp[0].output = 0;	
						inp[0].cus.account_number = 0;
						size = read(sock_desc, inp, sizeof(struct input)*2);

						if (inp[0].output == 1) {
							printf("\nusername already exists with an account. No more account can be created.\n");
							break;
						}
						else if(inp[0].output == 3) {
							printf("\nServer issue. No account is created\n");
							break;
						}
						else if(inp[0].output == 4) {
							printf("\nSingle account is created. Account number %d\n", inp[0].cus.account_number);
							break;
						}
					}
					break;

				case 2:
					printf("\n--------------Add a joint account--------------\n");

					for(int i=0; i<2; i++) {

						printf("\nEnter username for customer %d\n", i+1);
						size = read(0, &(inp[i].cus.username), sizeof(inp[i].cus.username));				
						inp[i].cus.username[size-1]='\0';

						printf("Enter password for customer %d\n", i+1);
						size = read(0, &(inp[i].cus.password), sizeof(inp[i].cus.password));				
						inp[i].cus.password[size-1]='\0';

						if(strlen(inp[i].cus.username)<1) {
							printf("\nusername cannot be empty.\n");
							break;
						}
						if(strlen(inp[i].cus.password)<1) {
							printf("\npassword cannot be empty\n");
							break;
						}
						if(strlen(inp[i].cus.username)>20) {
							printf("\nusername length exceeds 20!.\n");
							break;
						}
						if(strlen(inp[i].cus.password)>15) {
							printf("\npassword length exceeds 15!\n");
							break;
						}
					}
					inp[0].user_choice = 2;

					if (write(sock_desc, inp, sizeof(struct input)*2)==-1) {
						perror("\nUanble to send data to server");
						exit(EXIT_FAILURE);
					}				
					while(1) {
						inp[0].output = 0;	
						inp[0].cus.username[0]='\0';
						inp[1].cus.username[0]='\0';
						inp[0].cus.account_number = 0;
						size = read(sock_desc, inp, sizeof(struct input)*2);

						if (inp[0].output == 1) {
							printf("\nusername %s already exists with an account. No more account can be created.\n",inp[0].cus.username);
							break;
						}
						else if(inp[0].output == 2) {
							printf("\nusername %s already exists with an account. No more account can be created.\n",inp[1].cus.username);
							break;
						}
						else if(inp[0].output == 3) {
							printf("\nServer issue. No account is created\n");
							break;
						}
						else if(inp[0].output == 4) {
							printf("\nJoint account is created. Account number %d\n", inp[0].cus.account_number);
							break;
						}
					}
					break;
				case 3:
					printf("\n------------Delete a user------------\n");

					printf("\nEnter username\n");
					size = read(0, &(inp[0].cus.username), sizeof(inp[0].cus.username));				
					inp[0].cus.username[size-1]='\0';

					printf("Enter password\n");
					size = read(0, &(inp[0].cus.password), sizeof(inp[0].cus.password));				
					inp[0].cus.password[size-1]='\0';	

					if(strlen(inp[0].cus.username)<1) {
						printf("\nUsername cannot be empty\n");
						break;
					}
					if(strlen(inp[0].cus.password)<1) {
						printf("\nPassword cannot be empty.\n");
						break;
					}
					inp[0].user_choice = 3;

					if (write(sock_desc, inp, sizeof(struct input)*2)==-1) {
						perror("\nUnable to send data to server. Please try again.");
						break;
					}

					while(1) {
						inp[0].output = 0;	
						size = read(sock_desc, inp, sizeof(struct input)*2);
						
						if (inp[0].output == 1) {
							printf("\nInvalid username or user account information no longer persists.\n");
							break;
						}
						else if(inp[0].output == 2) {
							printf("\nPassword is wrong.\n");
							break;
						}
						else if(inp[0].output == 3) {
							printf("\nServer issue. User is not deleted.\n");
							break;
						}
						else if(inp[0].output == 4)
							break;
					}
					if (inp[0].acc.type == 'S' && inp[0].acc.balance > 0) {

						char admin_choice;
						printf("\nCustomer has a standing balance of %.2lf Rs. Do you really want to delete the customer? (Y/N)\n",inp[0].acc.balance);
						printf("\nEnter your choice\n");

						read(0, &admin_choice, 1);
						
						if (admin_choice != 'Y') {
	
							inp[0].user_choice = 1;
							write(sock_desc, inp, sizeof(struct input)*2);
							break;	
						}
					}
					
					if (write(sock_desc, inp, sizeof(struct input)*2)==-1) {
						perror("\nUnable to send data to server. Please try again.");
						break;
					}
					while(1) {
						inp[0].output = 0;	
						size = read(sock_desc, inp, sizeof(struct input)*2);

						if (inp[0].output == 1) {
							printf("\ncustomer is alreay deleted from database. Present delete operation fails.\n");
							break;
						}
						else if(inp[0].output == 2	) {
							printf("\nServer issue. Customer deletion failed.\n");
							break;
						}
						else if(inp[0].output == 3) {
							printf("\nCustomer %s is deleted from system.\n", inp[0].cus.username);
							break;							
						}
					}					
					break;		
				case 4:
					printf("\n------------Modify a user (password)------------\n");

					printf("\nEnter username\n");
					size = read(0, &(inp[0].cus.username), sizeof(inp[0].cus.username));				
					inp[0].cus.username[size-1]='\0';

					printf("Enter current password\n");
					size = read(0, &(inp[0].cus.password), sizeof(inp[0].cus.password));				
					inp[0].cus.password[size-1]='\0';	

					printf("Enter new password\n");
					size = read(0, &(inp[1].cus.password), sizeof(inp[1].cus.password));				
					inp[1].cus.password[size-1]='\0';	

					char password[17];

					printf("Confirm new password\n");
					size = read(0, password, sizeof(password));				
					password[size-1]='\0';	

					if(strlen(inp[1].cus.password)<1) {
						printf("\nNew password cannot be empty\n");
						break;
					}
					if(strlen(inp[1].cus.password)>15) {
						printf("\nNew password length exceeds 15!.\n");
						break;
					}
					if(strcmp(inp[1].cus.password, password)!=0) {
						printf("\nPassword did not match.\n");
						break;
					}

					inp[0].user_choice = 4;

					if (write(sock_desc, inp, sizeof(struct input)*2)==-1) {
						perror("\nUnable to send data to server. Please try again.");
						break;
					}
					
					while(1) {
						inp[0].output = 0;	
						size = read(sock_desc, inp, sizeof(struct input)*2);

						if (inp[0].output == 1) {
							printf("\nInvalid username.\n");
							break;
						}
						else if(inp[0].output == 2) {
							printf("\nExisting password did not match.\n");
							break;
						}
						else if(inp[0].output == 3) {
							printf("\nServer issue. Password did not updated\n");
							break;
						}
						else if(inp[0].output == 4) {
							printf("\nPassword successfully updated for user %s\n", inp[0].cus.username);
							break;
						}
					}
					break;					
				case 5:
					printf("\n------------Search for an account------------\n");

					printf("\nEnter account number of user\n");
					scanf("%d",&inp[0].cus.account_number);

					inp[0].user_choice = 5;

					if (write(sock_desc, inp, sizeof(struct input)*2)==-1) {

						perror("\nUnable to send data to server. Please try again.");
						break;
					}
					
					while(1) {
						inp[0].output = 0;	
						size = read(sock_desc, inp, sizeof(struct input)*2);

						if (inp[0].output == 1) {
							printf("\nInvalid account number.\n");
							break;
						}
						else if(inp[0].output == 2) {
							printf("\nServer issue. Account details cannot be displayed.\n");
							break;
						}
						else if(inp[0].output == 3) {
							printf("\n---------------Account information---------------\n");
							printf("\nAccount number:   %d\n", inp[0].cus.account_number);
							printf("\nAccount type:\t");

							switch(inp[0].acc.type) {

								case 'S':
									printf("Single account\n");
									printf("\nUsername:\t%s\n", inp[0].cus.username);
									break;
								case 'J':
									printf("Joint account\n");
									printf("\nUsername of first user:   %s\n", inp[0].cus.username);
									printf("\nUsername of second user:  %s\n", inp[1].cus.username);
							}
							switch(inp[0].cus.type) {

								case 'U':
									printf("\nUser type:	Ordinary customer\n");
									break;
								case 'A':
									printf("\nUser type:	Admin\n");
							}

							printf("\nAccount balance:   %.2lf Rs.\n",inp[0].acc.balance);
							break;
						}
					}
					break;									
				case 6:
					inp[0].user_choice = 6;

					if (write(sock_desc, inp, sizeof(struct input)*2)==-1)
					{
						perror("\nUanble to send data to server");
						exit(EXIT_FAILURE);
					}
					close(sock_desc);
					exit(EXIT_SUCCESS);
				default:
					printf("\nWrong choice. Please try again\n");
			}		
		}
	}
	else if(cust.type == 'U' ) {
		
		printf("\n***********************Welcome %s***********************\n",cust.username);
		struct input inp[2];
		ssize_t size;
		struct transaction_entry tran_entry;
		bool next;

		while(1) {
			int choice = showCustomerOption(cust.username);
			
			switch(choice) {
				case 1:
					// Deposit money
					printf("\n------------Deposit money------------\n");
					printf("\nEnter amount\n");
					scanf("%lf",&inp[0].acc.balance);

					if (inp[0].acc.balance<=0.00) {
						printf("\nPlease deposit a positive amount.\n");
						break;
					}

					strcpy(inp[0].cus.username,cust.username);
					inp[0].acc.account_number = cust.account_number;
					inp[0].user_choice = 1;

					if (write(sock_desc, inp, sizeof(struct input)*2)==-1) {
						perror("\nUnable to send data to server. Please try again.");
						break;
					}
					while(1) {
						inp[0].output = 0;	
						read(sock_desc, inp, sizeof(struct input)*2);

						if (inp[0].output == 1) {
							printf("\nEither your username or the account information are deleted from database.\n");
							break;
						}
						else if(inp[0].output == 2) {
							printf("\nTransaction failure. Cannot deposit money.\n");
							break;
						}
						else if(inp[0].output == 3) {
							printf("\nTransaction sucessful!\nAccount balance:   %.2lf Rs.\n",inp[0].acc.balance);
							break;
						}
					}
					break;
				case 2:
				// Withdraw money
					printf("\n------------Withdraw money------------\n");
					printf("\nEnter amount\n");
					scanf("%lf",&inp[0].acc.balance);

					if (inp[0].acc.balance<=0.00) {
						printf("\nPlease withdraw a positive amount.\n");
						break;
					}

					strcpy(inp[0].cus.username,cust.username);
					inp[0].acc.account_number = cust.account_number;
					inp[0].user_choice = 2;

					if (write(sock_desc, inp, sizeof(struct input)*2)==-1) {
						perror("\nUnable to send data to server. Please try again.");
						break;
					}
					while(1) {
						inp[0].output = 0;	
						read(sock_desc, inp, sizeof(struct input)*2);

						if (inp[0].output == 1) {
							printf("\nEither your username or the account information are deleted from database.\n");
							break;
						}
						else if(inp[0].output == 2) {
							printf("\nTransaction failure. Cannot deposit money.\n");
							break;
						}
						else if(inp[0].output == 3) {
							printf("\nTransaction sucessful!\nAccount balance:   %.2lf Rs.\n",inp[0].acc.balance);
							break;
						}
						else if(inp[0].output == 4) {
							printf("\nInsufficient balannce. Cannot withdraw money.\n");
							break;
						}
					}
					break;
				case 3:
					// Balance enquiry
					strcpy(inp[0].cus.username,cust.username);
					inp[0].acc.account_number = cust.account_number;
					inp[0].user_choice = 3;

					if (write(sock_desc, inp, sizeof(struct input)*2)==-1) {

						perror("\nUnable to send data to server. Please try again.");
						break;
					}
					
					while(1) {
						inp[0].output = 0;	
						read(sock_desc, inp, sizeof(struct input)*2);

						if (inp[0].output == 1) {
							printf("\nEither username or their account information are deleted from database.\n");
							break;
						}
						else if(inp[0].output == 2) {
							printf("\nServer issue. Cannot display balance now.\n");
							break;
						}
						else if(inp[0].output == 3) {
							printf("\nAccount balance:   %.2lf Rs.\n",inp[0].acc.balance);
							break;
						}
					}
					break;			
				case 4:
					// Password change
					printf("\n------------Change password------------\n");

					strcpy(inp[0].cus.username,cust.username);

					printf("\nEnter current password\n");
					size = read(0, &(inp[0].cus.password), sizeof(inp[0].cus.password));				
					inp[0].cus.password[size-1]='\0';	

					printf("Enter new password\n");
					size = read(0, &(inp[1].cus.password), sizeof(inp[1].cus.password));				
					inp[1].cus.password[size-1]='\0';	

					char password[17];

					printf("Confirm new password\n");
					size = read(0, password, sizeof(password));				
					password[size-1]='\0';	

					if(strlen(inp[1].cus.password)<1) {
						printf("\nNew password cannot be empty\n");
						break;
					}
					if(strlen(inp[1].cus.password)>15) {
						printf("\nNew password length exceeds 15!.\n");
						break;
					}
					if(strcmp(inp[1].cus.password, password)!=0) {
						printf("\nPassword did not match.\n");
						break;
					}

					inp[0].user_choice = 4;

					if (write(sock_desc, inp, sizeof(struct input)*2)==-1) {
						perror("\nUnable to send data to server. Please try again.");
						break;
					}
					
					while(1) {
						inp[0].output = 0;	
						read(sock_desc, inp, sizeof(struct input)*2);

						if (inp[0].output == 1) {
							printf("\nusername information are deleted from database.\n");
							break;
						}
						else if(inp[0].output == 2) {
							printf("\nExisting password did not match.\n");
							break;
						}
						else if(inp[0].output == 3) {
							printf("\nServer issue. Password is not updated\n");
							break;
						}
						else if(inp[0].output == 4) {
							printf("\nPassword is successfully updated.\n");
							break;
						}
					}
					break;				
				case 5:
					//View passbook
					inp[0].cus.account_number = cust.account_number;

					inp[0].user_choice = 5;

					if (write(sock_desc, inp, sizeof(struct input)*2)==-1) {
						perror("\nUnable to send data to server. Please try again.");
						break;
					}				
					while(1) {
						inp[0].output = 0;	
						read(sock_desc, inp, sizeof(struct input)*2);

						if(inp[0].output == 2) {
							printf("\nServer issue. Account details cannot be displayed.\n");
							break;
						}
						else if(inp[0].output == 3) {
							printf("\n---------------Account information---------------\n");
							printf("\nAccount number:   %d\n", cust.account_number);
							printf("\nAccount type:\t");

							switch(inp[0].acc.type) {

								case 'S':
									printf("Single account\n");
									printf("\nUsername:\t%s\n", inp[0].cus.username);
									break;
								case 'J':
									printf("Joint account\n");
									printf("\nUsername of first user:   %s\n", inp[0].cus.username);
									printf("\nUsername of second user:  %s\n", inp[1].cus.username);
							}
							printf("\nAccount balance:   %.2lf Rs.\n",inp[0].acc.balance);

							printf("\n---------------Transactions details---------------\n\n");

							while(1) {
								tran_entry.flag = 0;	
								read(sock_desc, &tran_entry, sizeof(tran_entry));

								if (tran_entry.flag == 1) {
									printf("\nServer failure. Cannot display any more transactions\n");
									break;
								}
								else if(tran_entry.flag == 2) {
									printf("\n-----------------------End-----------------------\n");
									break; 
								}
								else if(tran_entry.flag == 3) {
									printf("%02d-%02d-%02d    |    %02d:%02d:%02d    |    ",
									tran_entry.tran.day, tran_entry.tran.month, tran_entry.tran.year,
									tran_entry.tran.hour, tran_entry.tran.min, tran_entry.tran.sec);

									if(tran_entry.tran.mode == 'C')
										printf("Credit    |");
									else if(tran_entry.tran.mode == 'D')
										printf("Debit     |");

									printf("    %.2lf Rs.\n", tran_entry.tran.amount); 
									next = true;

									if (write(sock_desc, &next, sizeof(bool))==-1) {
										perror("Client request is not sent to server. Cannot display any more transactions\n");
										break;
									}
								}
							}
							break;
						}
					}
					break;					
				case 6:	
					// Exit
					inp[0].user_choice = 6;

					if (write(sock_desc, inp, sizeof(struct input)*2)==-1) {
						perror("\nUanble to send data to server");
						continue;
					}
					close(sock_desc);
					exit(EXIT_SUCCESS);	
				default:
					printf("\nWrong choice. Please try again\n");	
			}		
		}
	}
}

void showLoginPrompt(int sd, struct customer *cust) {

	struct login user;

	ssize_t len;

	while(1) {

		printf("\n***********************Welcome to Banking system***********************\n");
		printf("\nPlease enter your credentials\n\n");
		printf("username:  ");
		fgets(user.username, 22, stdin);
		printf("password:  ");
		fgets(user.password, 17, stdin);
		
		len = strlen(user.username)-1;

		if(user.username[len] == '\n') 
			user.username[len] = '\0';

		len = strlen(user.password)-1;

		if(user.password[len] == '\n')
			user.password[len] = '\0';

		if (strlen(user.username) == 0)
			printf("\nusername can't be blank\n");
		else if(strlen(user.password) == 0)
			printf("\nPassword can't be blank\n");
		else
			break;
	} 

	if (write(sd, &user, sizeof(user))==-1)
	{
		perror("\nUanble to send data to server");
		exit(EXIT_FAILURE);
	}

	while(1) {
		cust->account_number = 0;
		read(sd, cust, sizeof(struct customer));

		if (cust->account_number!=0)
			break;
	}
}

int showAdminOption(char *username) {

	printf("\n---------------MENU for %s---------------\n",username);
	printf("\n1. Add a single account");
	printf("\n2. Add a joint account");
	printf("\n3. Delete a user");
	printf("\n4. Modify a user (password)");
	printf("\n5. Search for an account");
	printf("\n6. Exit\n");
	printf("\nEnter a choice (1-6):\n");

	int choice;

	scanf("%d",&choice);

	return choice;
}

int showCustomerOption(char *username) {

	printf("\n---------------MENU for %s---------------\n",username);
	printf("\n1. Deposit money");
	printf("\n2. Withdraw money");
	printf("\n3. Check account balance");
	printf("\n4. Change password");
	printf("\n5. View passbook");
	printf("\n6. Exit\n");
	printf("\nEnter a choice (1-6):\n");

	int choice;

	scanf("%d",&choice);

	return choice;
}
