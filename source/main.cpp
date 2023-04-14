#include <iostream> // Import the input/output stream library.
#include <iomanip> // Import the input/output manipulation library.
#include <vector> // Import the vector container library.
#include <string> // Import the string container library.
#include <unistd.h> // Import the standard symbolic constants and types library.
#include <3ds.h> // Import the 3DS homebrew library.

#define SECOND(x) (x*1000ULL*1000ULL*1000ULL) // Define a macro for converting seconds to nanoseconds.
#define SYSTEMINFO_TYPE_CFWINFO 0x10000 // Define a constant for the type of system information.
#define CFWINFO_PARAM_ISRELEASE 0x200 // Define a constant for checking if the system is a release version.

PrintConsole topScreen, bottomScreen; // Define two console objects for the top and bottom screens.

using namespace std; // Use the standard namespace.

void initServices(); // Declare a function to initialize the services.
void exitServices(); // Declare a function to exit the services.
void fucked(); // Declare a function to handle errors.


void fucked() {
	cout << "\n\nPress [Start] to exit";	 // Output a message to the console.
	while (1) { // Infinite loop to wait for user input.
		hidScanInput(); // Scan for user input.
		if (hidKeysDown() & KEY_START) {  // If the user presses the start button.
			exitServices(); // Exit the services.
			exit(0); // Exit the program.
		}
	}
}


void initServices() { // Define the function to initialize the services.
	gfxInitDefault(); // Initialize the graphics subsystem.
	consoleInit(GFX_TOP, &topScreen); // Initialize the top screen console.
	consoleInit(GFX_BOTTOM, &bottomScreen); // Initialize the bottom screen console.
	consoleSelect(&bottomScreen); // Set the bottom screen console as the active console.
	cout << "\nTWLFix v1.3!\n\n"; // Output a message to the console.

	cout << "Initializing APT services\n"; // Output a message to the console.
	if (R_FAILED(aptInit())) { // If the APT services initialization fails.
		cout << "Failed to initialize APT services\n"; // Output an error message to the console.
		fucked(); // Call the error handling function.
	}
	cout << "Initializing AM services\n"; // Output a message to the console.
	if (R_FAILED(amInit())) { // If the AM services initialization fails.
		cout << "Failed to initialize AM services\n"; // Output an error message to the console.
		fucked(); // Call the error handling function.
	}
	cout << "Initializing PTM services\n"; // Output a message to the console.
	if (R_FAILED(ptmSysmInit())) { // If the PTM services initialization fails.
		cout << "Failed to initialize PTM services\n"; // Output an error message to the console.
		fucked(); // Call the error handling function.
	}
	cout << "Initializing CFG services\n"; // Output a message to the console.
	if (R_FAILED(cfguInit())) { // If the CFG services initialization fails.
		cout << "Failed to initialize CFG services\n"; // Output an error message to the console.
		fucked(); // Call the error handling function.
	}
}
void exitServices() { // Define the function to exit the services.
		cfguExit(); // Exit CFG services.
		ptmSysmExit(); // Exit PTM services.
		amExit(); // Exit AM services.
		aptExit(); // Exit AM services.
		gfxExit(); // Exit graphics services.
}


int main(int argc, char* argv[]) // Define the main function.
{

	bool isN3ds = false; // Initialize isN3ds variable as false.
	initServices(); // Call the function to initialize services.
	APT_CheckNew3DS(&isN3ds); // Check if the console is a New 3DS model.
	u8 region=0; // Initialize region variable to 0.
	if (R_FAILED(CFGU_SecureInfoGetRegion(&region))) { // Get the region of the console.
		region=255; // If getting the region fails, set region to 255.
	}
	bool isRelease = (OS_KernelConfig->env_info & 0b1) != 0; // Check if the console is in release mode.
	if (!isRelease) { // If the console is not in release mode, warn the user.
		cout << "\n*****\nSystem might be a devkit\n*****\n\nRunning this on a devkit WILL ONLY BREAK THINGS!\n"; 
	}
	cout << "\nPress [A] to begin or [X] to Exit!\n\n"; // Prompt the user to start or exit.
	
	while (1) { // Loop until the user presses A or Start.
		hidScanInput(); // Scan for user input.
		if (hidKeysDown() & KEY_A) { break; } // If A is pressed, exit the loop.
		if (hidKeysDown() & KEY_X) {  // If X is pressed, exit the program.
			exitServices();
			return 0;
		}
	}


	cout << "Uninstalling System DSiWare:\n"; // Inform the user that DSiWare is being uninstalled.
	
	vector<std::pair<std::string,u64>> Breakables =  { // Create a vector of title IDs and names to be uninstalled.
		std::make_pair("Whitelist",0x0004800f484e4841),		// Whitelist
		std::make_pair("Version Data",0x0004800f484e4C41),		// Version Data
		std::make_pair("DS Internet",0x0004800542383841),		// DS Internet
		std::make_pair("DS Download Play",0x00048005484E4441)		// DS Dlp
	};
	if (isN3ds) { // If the console is a New 3DS, add the twlfirm title ID to the vector.
		Breakables.push_back(std::make_pair("TWL Firm (n3DS)",0x0004013820000102));		// twlfirm n3ds

	}else{ // If the console is not a New 3DS, add the twlfirm title ID to the vector.
		Breakables.push_back(std::make_pair("TWL Firm (o3DS)",0x0004013800000102));	// twlfirm o3ds
	}
	if (region==4 || region==255) { // If the console is in China or the region cannot be determined, add the DS Dlp title ID to the vector.
		Breakables.push_back(std::make_pair("DS Download Play (CHN)",0x00048005484E4443));		// DS Dlp
	}else if (region==5 || region==255) { // If the console is in Korea or the region cannot be determined, add the DS Dlp title ID to the vector/
		Breakables.push_back(std::make_pair("DS Download Play (KOR)",0x00048005484E444B));		// DS Dlp
	}
// Print the vector of breakable titles.
	for (vector<std::pair<std::string,u64>>::iterator title=Breakables.begin();title != Breakables.end(); ++title) {
		cout << "Uninstalling " << (*title).first << "\t";
		if (R_FAILED(AM_DeleteTitle(MEDIATYPE_NAND, (*title).second))) {
			cout << "Failed\n";
		}else{
			cout << "Success\n";
		}
	}

	cout<<"\nDone!\nReboot and then open System Update.\n\nPress Start to reboot.\n";

	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START) break; 
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}
	PTMSYSM_RebootAsync(0);
	exitServices();

	return 0;
}
