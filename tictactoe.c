#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

//May not need
#include <linux/fs.h>

/*  Only for old linux, use linux/uacces.h
#include <asm/uaccess.h>
*/
#include <linux/uaccess.h>
#include <linux/slab.h> /*Apparently need for kmalloc*/
#include <linux/miscdevice.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Phillip Chow");	
MODULE_DESCRIPTION("Tic Tac Toe Game");

#define MAX_BUFFER_READ 7 /*Max number of characters going to read in "02_X_Y\n"*/


/*Return String Constants*/
#define UNKNOWN_CMD "UNKCMD\n"
#define BAD_FORMAT "INVFMT\n"
#define OK "OK\n"
#define WIN "WIN\n"
#define TIE "TIE\n"
#define OUT_OF_TURN "OOT\n"
#define ILL_MOVE "ILLMOVE\n"
#define NO_GAME "NOGAME\n"

#define MOVE_X 1
#define MOVE_O -1
#define MOVE_NONE 0

#define BAD_MOVE -1
#define VALID_MOVE 0
#define WIN_MOVE 1



/*Using this vide to start off of
 https://www.youtube.com/watch?v=CWihl19mJig
*/

/*Declare needed methods for kernal device
Then declare any other helper functions for tictactoe */



static int TicTacToe_open(struct inode* inode, struct file* filep); /*when device is opened?*/
static int TicTacToe_release(struct inode* inode, struct file* filep); /*when device is turned off?*/

static ssize_t TicTacToe_read(struct file* file, char __user * buffer, size_t length, loff_t* offset);/*How we read in input*/

static ssize_t TicTacToe_write(struct file* file, const char __user * buffer, size_t length, loff_t* offset); /*writing back to user?*/


static struct file_operations TicTacToe_fops =
{
	.owner = THIS_MODULE,
	.open = TicTacToe_open,
	.read = TicTacToe_read,
	.write = TicTacToe_write,
	.release = TicTacToe_release,
};

static struct miscdevice TicTacToe_dev = 
{   .minor = MISC_DYNAMIC_MINOR,     
	.name = "tictactoe",     
	.fops = &TicTacToe_fops,
	.mode = 0777,
};



static int* theBoard; /*handles the games state, initialize in init*/
static int turn; /*who's turn*/
static int playerTurn; /*Track what human is playing as*/
static int gameStart; /*Signifies if the game is running*/
static int turnsLeft; /*Track number of possible turns left*/
static char* returnString; /*for returning messages back to the user*/
static int numReturnString; /*For tracking size of return string*/

static char theBoardString[10]; //

static void printBoard(void);
static void clearBoard(void);
static int tryMove(int x, int y, int givenTurn);
static int tryMoveComp(int x, int givenTurn); /*Overloaded for computer's move*/


/*The basic module parts of the program*/

/*Everything done when we load the device*/
/*"TicTacToe" could be a constant instead of writing here*/
static int __init tictactoe_init(void)
{
	
	int returnCode;
	int iter;
printk(KERN_ALERT "Starting to init...\n");
/*	
	majorNumber = register_chrdev(0, "TicTacToe", &fops);
*/	
	/*recomended check if number not assigned*/
	printk(KERN_ALERT "Trying Misc_reg...\n");
	returnCode = misc_register(&TicTacToe_dev);	
	if( returnCode != 0)
	{
		printk(KERN_ALERT "ERROR! Couldn't register major number for device\n Major returned: %d \n", returnCode);
		return returnCode;
	}
		printk(KERN_ALERT "Success\n");


	printk(KERN_ALERT "Loading TicTacToe\n");
	
	/*Set up game board and variables*/
	turn = 0;
	playerTurn = 0;
	gameStart = 0;
	turnsLeft = 0;
	numReturnString = 0;
	
	returnString = (char*)kmalloc(10*sizeof(char),GFP_KERNEL);
	

	theBoard = (int*)kmalloc(9*sizeof(int),GFP_KERNEL);
	iter = 0;
	printk(KERN_ALERT "Initializing game board...\n");
	while(iter < 9)
	{
		printk(KERN_ALERT "Game board %i initializing\n", iter);
		theBoard[iter] = 0;

		theBoardString[iter] = '*';
		iter++;
	}

		theBoardString[9] = '\n';

	printk(KERN_ALERT "End of Init...\n");
	return 0;
}

/*Everything done when we remove the device (maybe free any data?)*/
/*again, "tictactoe" could be a constant we define*/
static void __exit tictactoe_exit(void)
{
	int returnCode;
	printk(KERN_ALERT "un-Loading TicTacToe\n");
	
	/*free anything made?*/
	printk(KERN_ALERT "freeing board...\n");
	kfree(theBoard);

	/* This may not be necessary, since just a pointer to either static constants, or theBoardString, which might get deleted next...
	printk(KERN_ALERT "(freeing return string\n");
	kfree(returnString);
	*/


	/* ...it won't. freeing either gives errors, this one gives seg fault. Guessing theBoardString is technically
		static data that was never made will kmalloc, so it will get deleted by default.
 	printk(KERN_ALERT "(freeing theBoardstring\n");
	kfree(theBoardString);
	*/
	printk(KERN_ALERT "de registering....\n");
	misc_deregister(&TicTacToe_dev);
	/*
	if(unreg <0)
		printk(KERN_ALERT "ERROR: Couldn't unload TicTacToe. Error code: %d\n", unreg);
	*/
printk(KERN_ALERT "done exit!\n");
}



/*The device functions*/

static int TicTacToe_open(struct inode* inode, struct file* filep)
{
	
	/*
	//when opened/called?
	//This may be where we need to use mutex/semaphore to lock this
	//to count how many processes are using for read/write
	//Hold off for now, read somewhere on Piazza Hut that locking not necessary for this assignment
	*/
	

	return 0;
}

static int TicTacToe_release(struct inode* indoe, struct file* filep)
{
	/*figure this one out, when unloaded
	Like said above, but maybe releasing mutex/semaphore*/
	
	return 0;
}

static ssize_t TicTacToe_read(struct file* file, char __user * buffer, size_t length, loff_t * offset)
{
	/*
	//Allow user to read the latest return string
	//use copy_to_user function the returnString
	*/


	int numRead;

printk(KERN_ALERT "Starting to read...\n");
	
	/* Hold for now, redundent for copy_from_ser 


	if(access_ok(VERIFY_WRITE, buffer, length) == 0)
	{
		
		return EFAULT;
	}
	*/
printk(KERN_ALERT "checking bad buffer and length...\n");
	if(buffer == NULL || length < 0)
	{
		return EFAULT;
	}
	
	if(length < numReturnString)
		numRead = length;
	else
		numRead = numReturnString;
	printk(KERN_ALERT "assigned lesser of read length...\n");
	
	/* unsigned char* copiedMessage = (char*)kmalloc(numRead,GFP_KERNEL); */
	printk(KERN_ALERT "trying copy to user...\n");
	if((copy_to_user(buffer, returnString, numRead)) != 0)
	{
		printk(KERN_ALERT "can't let user read, exiting...\n");
		/*Free copiedMessage, return error*/
		/*kfree(copiedMessage);*/
		
		/*Unlock any mutex if there is one*/
		return EFAULT;
	}
	
	/*		Nothing to do	*/
	printk(KERN_ALERT "copy to user /read success, closing...\n");
	
	/*Return number of read in bytes*/
	return numRead;
}

static ssize_t TicTacToe_write(struct file* file, const char __user * buffer, size_t length, loff_t * offset)
{
	int numRead;
	unsigned char* copiedMessage;
	int iter;
	int tempInt;
	int x;
	int y;

	int i;

printk(KERN_ALERT "Starting to write to device...\n");
	/*
	//Where user writes TO MODULE
	
	//first check pointers validity, can write, then copy_from_user
	*/
	

	printk(KERN_ALERT "check buffer null...\n");

	if(buffer == NULL)
	{
		printk(KERN_ALERT "..buffer null, return error\n");
		return EFAULT;
	}
	printk(KERN_ALERT "check if input length < 2...\n");
	/*Only valid inputs are at least 2 characters*/
	if(length < 2)
	{
		printk(KERN_ALERT "...input length too short, exiting!\n");
		return EFAULT; /*TODO Check if better Error*/
	}
	
	if(length < MAX_BUFFER_READ)
		numRead = length;
	else
		numRead = MAX_BUFFER_READ;
	printk(KERN_ALERT "assigned shorter wrtie length...\n");

	printk(KERN_ALERT "K RE allocating copied message...\n");
	copiedMessage = (unsigned char*)kmalloc(numRead, GFP_KERNEL); /*Will initialize if not already*/
	printk(KERN_ALERT "...Realloc success!...\n");

	printk(KERN_ALERT "checking if valid can copy from user...\n");
	if((copy_from_user(copiedMessage, buffer, numRead)) != 0)
	{
		printk(KERN_ALERT "...Invalid copy from user...\n");
		/*Free copiedMessage, return error*/
		printk(KERN_ALERT "freeing copied message...\n");
		kfree(copiedMessage);
		printk(KERN_ALERT "...exiting with error!\n");
		/*Unlock any mutex if there is one*/
		return EFAULT;
	}
	
	/*		PARSE, FIND out if correct command, Then do command	*/


	printk(KERN_ALERT "\n...Big Parser Part!\n");
	if(copiedMessage[0] != '0')
	{
		/*returnString = (char*)krealloc(returnString, 7, GFP_KERNEL);*/
		numReturnString = 7;
		returnString = UNKNOWN_CMD;
	}
	else
	{
		if(copiedMessage[1] == '0')
		{
			printk(KERN_ALERT "given code 00...\n");
			if(copiedMessage[2] != ' ')
			{
				printk(KERN_ALERT "BUT NO SPACE AFTER, EXITING!...\n");
				kfree(copiedMessage);
				/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
				numReturnString = 7;
				returnString = BAD_FORMAT;
				return numRead;
			}
			if(copiedMessage[3] == 'X' || copiedMessage[3] == 'x')
			{
				printk(KERN_ALERT "Setting player X...\n");
				tempInt = MOVE_X;
			}
			else if(copiedMessage[3] == 'O' || copiedMessage[3] == 'o')
			{
				printk(KERN_ALERT "Setting Player O...\n");
				tempInt = MOVE_O;
			}
			else
			{
				printk(KERN_ALERT "...Invalid player init character, exiting!!\n");
				kfree(copiedMessage);
				/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
				returnString = UNKNOWN_CMD;
				numReturnString = 7;
				return numRead;
			}

			/*Check if nothing else typed in*/
				printk(KERN_ALERT "Checking if remaining is space or new line...\n");
			iter = 4;
			while(iter < numRead)
			{
				if(copiedMessage[iter] != ' ' && copiedMessage[iter] != '\n')
				{
					printk(KERN_ALERT "...Character at %i is not space or newline, exiting!\n", iter);
					kfree(copiedMessage);
					/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
					returnString = UNKNOWN_CMD;
					numReturnString = 7;
					return numRead;
				}
				iter++;
			}
			printk(KERN_ALERT "Strating new game variables...\n");
			/*Start new Game*/
			clearBoard();
			gameStart = 1;
			turn = MOVE_X;
			playerTurn = tempInt;
			turnsLeft = 9;

			printk(KERN_ALERT "Setting return string...\n");
			/*returnString =(char*)krealloc(returnString, 3, GFP_KERNEL);*/
			returnString = OK;
			printk(KERN_ALERT "return message is...");
			printk(KERN_ALERT "%s",returnString);
			numReturnString = 3;
		}


		else if(copiedMessage[1] == '1')
		{
			printk(KERN_ALERT "Command is print board...\n");
			if(numRead >2) /*Check any remaining characters and ensure empty*/
			{
				iter = 2;
				while(iter < numRead)
				{
					if(copiedMessage[iter] != ' ' && copiedMessage[iter] != '\n')
					{
						printk(KERN_ALERT "...Character at %i is not space or newline, exiting!\n", iter);
						kfree(copiedMessage);
						/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
						returnString = BAD_FORMAT;
						numReturnString = 7;
						return numRead;
					}
					iter++;
				}
			}


			printBoard();


		}



		else if(copiedMessage[1] == '2')
		{
			printk(KERN_ALERT "Making move...\n");
			if(copiedMessage[2] != ' ')
			{
				printk(KERN_ALERT "...But NO SPACE AFTER COMMAND! Exiting\n");
				kfree(copiedMessage);
				/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
				returnString = UNKNOWN_CMD;
				numReturnString = 7;
				return numRead;
			}
			if(gameStart != 1)
			{
				printk(KERN_ALERT "...But GAME HASNT STARTED! Exiting\n");
				kfree(copiedMessage);
				/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
				returnString = NO_GAME;
				numReturnString = 7;
				return numRead;
			}
			if(playerTurn != turn)
			{
				printk(KERN_ALERT "...But Not TURN! Exiting\n");
				kfree(copiedMessage);
				/*returnString =(char*)krealloc(returnString, 4, GFP_KERNEL);*/
				returnString = OUT_OF_TURN;
				numReturnString = 4;
				return numRead;
			}


			printk(KERN_ALERT "Checking X cordinate from character 3...\n");
			if(!(copiedMessage[3] > 47 && copiedMessage[3] < 51))
			{
				printk(KERN_ALERT "...But not a valid Number! Exiting\n");
				kfree(copiedMessage);
				/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
				returnString = BAD_FORMAT;
				numReturnString = 7;
				return numRead;
			}
			if(copiedMessage[4] != ' ')
			{
				printk(KERN_ALERT "...NO Space After X! Exiting\n");
				kfree(copiedMessage);
				/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
				returnString = BAD_FORMAT;
				numReturnString = 7;
				return numRead;
			}
		printk(KERN_ALERT "Checking Y pos at character 5...\n");
			if(!(copiedMessage[5] > 47 && copiedMessage[5] < 51))
			{
				printk(KERN_ALERT "...But Not a Valid Number! Exiting\n");
				kfree(copiedMessage);
				/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
				returnString = BAD_FORMAT;
				numReturnString = 7;
				return numRead;
			}

			iter = 6;
			printk(KERN_ALERT "Checking if rest are spaces or newline...\n");
			while(iter < numRead)
			{
				if(copiedMessage[iter] != ' ' && copiedMessage[iter] != '\n')
				{
					printk(KERN_ALERT "...None valid Blank character at %i ! Exiting\n", iter);
					kfree(copiedMessage);
					/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
					returnString = BAD_FORMAT;
					numReturnString = 7;
					return numRead;
				}
				iter++;
			}

			/*Formatting is fine if here, assign x,y */
			x = copiedMessage[3] - '0';
			y = copiedMessage[5] - '0';

			printk(KERN_ALERT "Valid X and Y, you was given %i, %i, trying to make a move...\n", x, y);
			tempInt = tryMove(x, y, playerTurn);


			if(tempInt == WIN_MOVE)
			{
				printk(KERN_ALERT "Move Valid AND WIN!!!...\n");
				gameStart = 0;
				kfree(copiedMessage);
				/*returnString =(char*)krealloc(returnString, 4, GFP_KERNEL);*/
				returnString = WIN;
				numReturnString = 4;
				return numRead;
			}
			else if(tempInt == VALID_MOVE)
			{
				printk(KERN_ALERT "...Move valid\n");
				if(turnsLeft == 0)
				{		
					printk(KERN_ALERT "...And Tie!\n");
					gameStart = 0;
					kfree(copiedMessage);
					/*returnString =(char*)krealloc(returnString, 4, GFP_KERNEL);*/
					returnString = TIE;
					numReturnString = 4;
					return numRead;
				}
				else
				{
					printk(KERN_ALERT "...Ending input with OK!\n");
					printk(KERN_ALERT "Freeing Copied Message...!\n");
					kfree(copiedMessage);
					printk(KERN_ALERT "Changing Turns from %i...!\n", turn);
					turn = (-1) * turn;
					printk(KERN_ALERT "to %i...!\n", turn);
					printk(KERN_ALERT "!!! Trying to Realloc returnString...!\n");
					/*returnString =(char*)krealloc(returnString, 3, GFP_KERNEL);*/
					printk(KERN_ALERT "Supposedly reallocated...!\n");
					returnString = OK;
					numReturnString = 3;
					printk(KERN_ALERT "Returning message okNewline...!\n");
					return numRead;
				}
			}
			else
			{
				printk(KERN_ALERT "...That move was Illegal, closing\n");
				kfree(copiedMessage);
				/*returnString =(char*)krealloc(returnString, 8, GFP_KERNEL);*/
				returnString = ILL_MOVE;
				numReturnString = 8;
				return numRead;
			}
		}


		else if(copiedMessage[1] == '3')
		{
			printk(KERN_ALERT "making PC go...\n");
			printk(KERN_ALERT "Checking if no more inputs...\n");
			iter = 2;
			while(iter < numRead)
			{
				if(copiedMessage[iter] != ' ' && copiedMessage[iter] != '\n')
				{
					printk(KERN_ALERT "...Non Empty character at %i! Exiting\n", iter);
					kfree(copiedMessage);
					/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
					returnString = BAD_FORMAT;
					numReturnString = 7;
					return numRead;
				}
				iter++;
			}
			if(gameStart != 1)
			{
				printk(KERN_ALERT "...Game not started, Exiting\n");
				kfree(copiedMessage);
				/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
				returnString = NO_GAME;
				numReturnString = 7;
				return numRead;
			}
			if((playerTurn * (-1)) != turn)
			{
				printk(KERN_ALERT "...Not PC's Turn, Exiting!\n");
				kfree(copiedMessage);
				/*returnString =(char*)krealloc(returnString, 4, GFP_KERNEL);*/
				returnString = OUT_OF_TURN;
				numReturnString = 4;
				return numRead;
			}

	
			
			/*Keep it simple for assignment, just iterate through and find empty spot*/
			printk(KERN_ALERT "Pc is now brute force find move...\n");
			x = 0;
			while(x < 9)
			{
				tempInt = tryMoveComp(x, (playerTurn * (-1)));
				if(tempInt == WIN_MOVE)
				{
					printk(KERN_ALERT "...Brute Force move is PC WIN! Exiting!\n");
					gameStart = 0;
					kfree(copiedMessage);
					/*returnString =(char*)krealloc(returnString, 4, GFP_KERNEL);*/
					returnString = WIN;
					numReturnString = 4;
					return numRead;
				}
				else if(tempInt == VALID_MOVE)
				{
					if(turnsLeft == 0)
					{
						printk(KERN_ALERT "...Brute Force caused a TIE! Exiting\n");
						gameStart = 0;
						kfree(copiedMessage);
						/*returnString =(char*)krealloc(returnString, 4, GFP_KERNEL);*/
						returnString = TIE;
						numReturnString = 4;
						return numRead;
					}
					else
					{
						printk(KERN_ALERT "...Brute Force Move Success\n");
						kfree(copiedMessage);
						printk(KERN_ALERT "Changing Turns from %i...!\n", turn);
						turn = (-1) * turn;
						printk(KERN_ALERT "to %i...!\n", turn);
						/*returnString =(char*)krealloc(returnString, 3, GFP_KERNEL);*/
						returnString = OK;
						numReturnString = 3;
						return numRead;
					}
				}
				x++;
			}

		}

		else
		{
			printk(KERN_ALERT "...Don't know command, Eixiting\n");
			/*returnString =(char*)krealloc(returnString, 7, GFP_KERNEL);*/
			returnString = UNKNOWN_CMD;
			numReturnString = 7;
		}
	}
	
	printk(KERN_ALERT "Freeing Copiedd Message...\n");
	kfree(copiedMessage);
	printk(KERN_ALERT "...Free Success, Returning with num read in characters\n");
	/*Return number of read in bytes*/
	return numRead;
	
}


static void printBoard()
{
	int i;

	printk(KERN_ALERT "Printing Board...\n");
	i = 0;

	printk(KERN_ALERT "Setting up returnString for board...\n");

	while(i < 9)
	{
		printk(KERN_ALERT "while loop starts at i= %i \n",i);
		if(theBoard[i] == MOVE_X)
		{
			printk(KERN_ALERT "X\n");
			theBoardString[i] = 'X';
			/*returnString[i] = 'X';*/
		}
		else if(theBoard[i] == MOVE_O)
		{
			printk(KERN_ALERT "O\n");
			theBoardString[i] = 'O';
			/*returnString[i] = 'O';*/
		}
		else
		{
			printk(KERN_ALERT "*\n");
			theBoardString[i] = '*';
			/*returnString[i] = '*';*/
		}
		printk(KERN_ALERT "loop finished, trying to increase i\n");
		i++;
		printk(KERN_ALERT "loop finished, moving to i= %i \n",i);
	}

	returnString = theBoardString;

	printk(KERN_ALERT "\n Board done printing...");
	printk(KERN_ALERT "Setting ReturnStrinNum");
	numReturnString = 10;
	printk(KERN_ALERT "Setting last of return string newLine");
	returnString[9] = '\n';
	printk(KERN_ALERT "...Doen Printing Board\n");
} 
static void clearBoard()
{
	int i;
	printk(KERN_ALERT "Clearing Board...\n");
	i = 0;
	while(i < 9)
	{
		theBoard[i] = MOVE_NONE;
		i++;
	}
	
}
static int tryMove(int x, int y, int givenTurn)
{
	int index;
	printk(KERN_ALERT "In TryMove method...\n");
	index = (y*3) + x;
	printk(KERN_ALERT "X.%i, Y.%i, Index.%i , In TryMove method...\n", x, y, index);
	if(x < 0 || x > 2 || y < 0 || y > 2)
	{
		printk(KERN_ALERT "Bad Move, X and or Y not valid, either bigger or smaller than possible...\n");
		return BAD_MOVE;
	}
	else if(theBoard[index] != MOVE_NONE)
	{
		printk(KERN_ALERT "Bad move, Space taken already...\n");
		return BAD_MOVE;
	}
	else
	{
		printk(KERN_ALERT "Making Valid Move...\n");
		theBoard[index] = givenTurn;
		turnsLeft--;
	}
printk(KERN_ALERT "Checking if a Win...\n");
	if((theBoard[0] == givenTurn && theBoard[1] == givenTurn && theBoard[2] == givenTurn) ||
		(theBoard[3] == givenTurn && theBoard[4] == givenTurn && theBoard[5] == givenTurn) ||
		(theBoard[6] == givenTurn && theBoard[7] == givenTurn && theBoard[8] == givenTurn) ||
	(theBoard[0] == givenTurn && theBoard[3] == givenTurn && theBoard[6] == givenTurn) ||
		(theBoard[1] == givenTurn && theBoard[4] == givenTurn && theBoard[7] == givenTurn) ||
		(theBoard[2] == givenTurn && theBoard[5] == givenTurn && theBoard[8] == givenTurn) ||
	(theBoard[0] == givenTurn && theBoard[4] == givenTurn && theBoard[8] == givenTurn) ||
		(theBoard[2] == givenTurn && theBoard[4] == givenTurn && theBoard[6] == givenTurn)
	)
		return WIN_MOVE;
	else
		return VALID_MOVE;
}
static int tryMoveComp(int x, int givenTurn)
{
	int index;
	printk(KERN_ALERT "Computer trying move...\n");
	index = x;
	
	if(theBoard[index] != MOVE_NONE)
	{
		return BAD_MOVE;
	}
	else
	{
		theBoard[index] = givenTurn;
		turnsLeft--;
	}
	printk(KERN_ALERT "Computer found valid move, checking if win...\n");
	if((theBoard[0] == givenTurn && theBoard[1] == givenTurn && theBoard[2] == givenTurn) ||
		(theBoard[3] == givenTurn && theBoard[4] == givenTurn && theBoard[5] == givenTurn) ||
		(theBoard[6] == givenTurn && theBoard[7] == givenTurn && theBoard[8] == givenTurn) ||
	(theBoard[0] == givenTurn && theBoard[3] == givenTurn && theBoard[6] == givenTurn) ||
		(theBoard[1] == givenTurn && theBoard[4] == givenTurn && theBoard[7] == givenTurn) ||
		(theBoard[2] == givenTurn && theBoard[5] == givenTurn && theBoard[8] == givenTurn) ||
	(theBoard[0] == givenTurn && theBoard[4] == givenTurn && theBoard[8] == givenTurn) ||
		(theBoard[2] == givenTurn && theBoard[4] == givenTurn && theBoard[6] == givenTurn)
	)
		return WIN_MOVE;
	else
		return VALID_MOVE;
}

module_init(tictactoe_init);
module_exit(tictactoe_exit);

