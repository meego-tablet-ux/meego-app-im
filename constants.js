// this file contains the translatable strings of meego-app-im

// AccountDelegate.qml
var multipleAccountsTitle =  qsTr("Multiple accounts connected");
var multipleAccountsText = qsTr("Do you really want to connect this account? By doing this all other %1 accounts will be disconnected.");
var accountLogout = qsTr("Log out");
var accountLogin = qsTr("Log in to %1");
var accountSettings = qsTr("Settings");

// AccountScreenContent
var accountList = qsTr("Account list");
var accountsLoading = qsTr("Loading accounts...");
var accountChoose = qsTr("Choose an account");
var accountAdd = qsTr("Add an account");

// AddAFriend.qml
var friendUsername = qsTr("Friend's username");
var friendSend = qsTr("Send");
var friendSendingRequest = qsTr("Sending request");
var friendRequestSent = qsTr("Request sent");
var friendNoNetwork = qsTr("Your device is not connected to a network. Please connect and try again.")

// ContactContentMenu.qml
var contactUpdateStatus = qsTr("Update status");
var contactYourStatus = qsTr("Your Status:");
var contactCustomStatusMessage = qsTr("Custom status message");
var contactStatusUpdate = qsTr("Update");
var contactChangeDisplayName = qsTr("Change display name");
var contactChangeDisplayNameText = qsTr("Display name");
var contactDisplayNameUpdate = qsTr("Update");
var contactAddFriend = qsTr("Add a friend");
var contactClearChatHistory = qsTr("Clear chat history");
var contactLogin = qsTr("Log in");
var contactLogout = qsTr("Log out");

// ContactDelegate.qml
var contactReturnToChat = qsTr("Return to chat");
var contactOpenChat = qsTr("Open chat");
var contactCall = qsTr("Call")
var contactVideoCall = qsTr("Video call")
var contactUnblock = qsTr("Unblock");
var contactBlock = qsTr("Block");
var contactEndChat =  qsTr("End chat");
var contactDeleteContact = qsTr("Delete contact");

// ContactPickerContent.qml
var contactPickerAddToChat = qsTr("Add contacts to chat");
var contactPickerAdd = qsTr("Add");
var contactPickerCancel = qsTr("Cancel");

// ContactRequestDelegate.qml
var contactRequestFrom = qsTr("Friend request from");
var contactRequestAccept = qsTr("Accept");
var contactRequestCancel = qsTr("Cancel");

// ContactScreenContent.qml
var contactScreenAddFriend = qsTr("Add a friend");
var contactScreenLoading = qsTr("Loading contacts...");
var contactScreenNoFriends = qsTr("You haven't added any friends yet");
var contactScreenAccountOffline = qsTr("Account is offline");
var contactScreenAccountConnecting = qsTr("Logging in...");

// FileTransferDelegate.qml
var fileTransferOpen = qsTr("Open");
var fileTransferProblemDownloading = qsTr("There was a problem downloading");
var fileTransferProblemUploading = qsTr("There was a problem uploading");
var fileTransferCanceled = qsTr("Canceled");
var fileTransferSave = qsTr("Save");
var fileTransferDecline = qsTr("Decline");
var fileTransferCancel = qsTr("Cancel");

// MessageContactDelegate.qml
var messageContactPrivateChat = qsTr("Private chat");
var messageContactAdd = qsTr("Add to contacts");

// MessageContentMenu.qml
var messageAddContacts = qsTr("Add contacts to chat");
var messageClearChatHistory = qsTr("Clear chat history")
var messageEndChat = qsTr("End chat");

// MessageDelegate.qml
//: %1 is the file size
var messageFileSize = qsTr("(%1)");
var messageUploadCanceled = qsTr("Upload canceled:");
var messageFileSent = qsTr("Sent:");
var messageFileUploading = qsTr("Uploading:");
//: %1 is the nickname of the contact sending you a file
var messageFileSentToYou = qsTr("%1 has sent you:");
//: %1 is the nickname of the contact sending you a file
var messageFileSendingToYou = qsTr("%1 is sending you:");
//: %1 is event describing what happened - %2 is date and time
var messageCustomEvent = qsTr("%1 - %2");
//: %1 contact id who called - %2 date time of the call
var messageTriedCall = qsTr("%1 tried to call - %2");
//: %1 contact id who rejected the call - %2 date time of the call
var messageRejectedCall = qsTr("%1 rejected call - %2");
//: %1 contact id who called - %2 duration of the call - %3 date time of the call
var messageCalled = qsTr("%1 called - duration %2 - %3");

// MessageScreenContent.qml
var messageOpeningChat = qsTr("Opening chat...");
var messageLoadingHistory = qsTr("Loading conversation history...");
var messageSend = qsTr("Send");
var messageGroupConversation = qsTr("Group conversation");
//: %1 is the nickname of the contact you are chatting with
var messageChatWith = qsTr("Chat with %1");

// NoNetworkHeader.qml
var noNetworkText = qsTr("Sorry, we can't login because it is not connected to a network. Go to Wireless & Network settings and connect to an available network");

// PictureChangeMenu.qml
var pictureChangeTitle = qsTr("Change your picture:");
var pictureTake = qsTr("Take picture")
var pictureChoose = qsTr("Choose picture")

// SearchHeader.qml
//: %1 is the number of items matching the search criteria
var searchMatches = qsTr("%1 matches found");
var searchOlder = qsTr("Older");
var searchNewer = qsTr("Newer");

// SendFileView.qml
var sendFileTitle = qsTr("Send:");
var sendFilePhoto = qsTr("Photo");
var sendFileVideo = qsTr("Video");
var sendFileMusic = qsTr("Music");
var sendFileContact = qsTr("Contact Details");

// VideoWindow.qml
//: %1 is the nickname of the contact you are in a call with
var videoWindowCallWith = qsTr("Call with %1");
var videoWindowMinimize = qsTr("Minimize me") ;
var videoWindowMaximize = qsTr("Maximize me");
var videoWindowDisableCamera = qsTr("Disable camera");
var videoWindowEnableCamera = qsTr("Enable camera");
var videoWindowSwapCamera = qsTr("Swap camera");

// main.qml
var mainChat = qsTr("Chat");
var mainAccountSwitcher = qsTr("Account switcher");
var mainStatusAvailable = qsTr("Available");
var mainStatusBusy = qsTr("Busy");
var mainStatusAway = qsTr("Away");
var mainStatusExtendedAway = qsTr("Extended away");
var mainStatusOffline = qsTr("Offline");
var mainStatusInvisible = qsTr("Invisible");
var mainSelectContact

// Error messages
var errorRetypeUserPassword = qsTr("Sorry, there was a problem logging in. Please retype your username and password.");
var errorLoginAccount = qsTr("Sorry, there was a problem logging in to %1. Please go to Settings and retype your username and password.");
var errorLogin = qsTr("Sorry, there was a problem logging in. Please go to Settings and retype your username and password.");
var errorLoginAccountTryLater = qsTr("Sorry, there was a problem logging in to %1. Please try again later.");
var errorLoginTryLater = qsTr("Sorry, there was a problem logging in. Please try again later.");
var errorLogoutAccountConnectedElse = qsTr("It looks like you have logged in to %1 from somewhere else, so we have logged you out from this tablet. Try logging in again");
var errorLogoutConnectedElse = qsTr("It looks like you have logged in to this account from somewhere else, so we have logged you out from this tablet. Try logging in again");
var errorEncryptionDeselect = qsTr("Sorry, there was a problem logging in. Deselecting the 'Encryption required' option in Advanced settings should solve this problem.");
var errorEncryptionAccountDeselect = qsTr("Sorry, there was a problem logging in to %1. Deselecting the 'Encryption required' option in Advanced settings should solve this problem.");
var errorSslError = qsTr("Sorry, there was a problem logging in. Selecting the 'Ignore SSL certificate errors' option in the account Advanced settings should solve this problem.");
var errorSslAccountError = qsTr("Sorry, there was a problem logging in to %1. Selecting the 'Ignore SSL certificate errors' option in the account Advanced settings should solve this problem.");
