#############################################################
#
#	Property of HABET.
#
#   Authors:
#           Jared Danner
#
#############################################################
from login_functions import *
from gui_functions import *
from communication import *

# Enables / Disables login window.
LOGIN_ENABLE = True


def main():
	"""  Main method of GUI. """

	# Checks for login method.
	if LOGIN_ENABLE:
		# Launches the login terminal.
		login()
		# Launches the main GUI terminal.
		GUI()
	else:
		# Launches the main GUI terminal.
		GUI()


def login():
	"""  Login Screen Config and Generation """

	# Creation of class.
	login = Login_Terminal()
	# Configures and displays the login window.
	login.configure_login_window()
	# Checks for potential manual termination of the window.
	if login.lock_state is True:
		# Gives verbose error message in cmd / terminal.
		print("User gave invalid credentials / closed the window.")
		# Shuts down the application
		quit()


def GUI():
	"""
	Main terminal application for interfacing
	with the craft.
	"""
	# Creation of class.
	gui = GUI_Terminal()
	# Configures and displays the application window.
	gui.configure_gui_terminal()


# Forces script to start at the method main().
if __name__ == "__main__":
   main()
