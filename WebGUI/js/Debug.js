//=====================================================================================
//
//	Created Dec, 2012
//	by Ryan Rivera ((rrivera at fnal.gov))
//
//	Debug.js
//
//	Since different browser have different console print statements, all ots code
// 		should use Debug.log(str,num[optional, default=0]). Where str is the string to print to console and
// 		num is the priority number 0 being highest.
//
//	Debug.log() will print to console if Debug.mode = 1 and if num < Debug.level
//
//=====================================================================================

var Debug = Debug || {}; //define Debug namespace

Debug.mode = 1; 		//0 - debug off, 1 - debug on
Debug.simple = 0; 		//0 - use priority (more detail in printout), 1 - simple, no priority
Debug.level = 0;		//priority level, all logs with lower priority level are printed

//setup default priorities
Debug.HIGH_PRIORITY = 0;
Debug.MED_PRIORITY = 50;
Debug.LOW_PRIORITY = 100;


if (Debug.mode) //IF DEBUG MODE IS ON!
{
	if (Debug.simple)
	{
		//If want default console.log use this:
		Debug.log = console.log.bind(window.console);
	}
	else
	{
		//For fancy priority management use this:
		Debug.log = function(str,num=Debug.LOW_PRIORITY) { 	//make num optional and default to lowest priority	
				if(Debug.level < 0) Debug.level = 0; //check for crazies, 0 is min level
				if(Debug.mode && num <= Debug.level)
				{				
					
					console.log("Priority-" + num + 
							 ":\t " + (new Error).stack.split("\n")[2] + ":\n\t" +
							str); 
					//console.log("" + str, "\n\n===   Debug Priority " + num + "   ===\n\t" + 
					//		(new Error).stack.split("\n")[2]); //gets calling source from call stack
				}
			}
	}
}
else	//IF DEBUG MODE IS OFF!
{	//do nothing with log functions
	console.log = function(){}
	Debug.log = function(){}
}

// living and breathing examples:
Debug.log("Debug mode is on at level: " + Debug.level);
Debug.log("This is an example for posterity that is not printed due to debug priority.",Debug.level+1);

