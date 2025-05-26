#include <iostream>
#include <string>
#include <ctime>           // for showing local time
#include <limits>
#include <cstdlib>          // for random generating
using namespace std;

// Utility function to get the current time
string getCurrentTime() {
    time_t now = time(0);
    tm localtm;
    localtime_s(&localtm, &now);
    char buffer[80];
    strftime(buffer, 80, "%d-%m-%Y %H:%M:%S", &localtm);
    return string(buffer);
}

// trree (BST)
struct TreeNode {
    string name;
    bool isFile;
    string content;
    TreeNode* left;
    TreeNode* right;
    TreeNode* parent;

    TreeNode(const string& nodeName, bool file = false)
        : name(nodeName), isFile(file), content(""), left(nullptr), right(nullptr), parent(nullptr) {
    }
};

// Binary Search Tree for the File System
class FileSystemTree {
private:
    TreeNode* root;
    TreeNode* currentDir;

    //   delete the entire tree
    void deleteTree(TreeNode* node) {
        if (!node) return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }

    // function to find a node by name
    TreeNode* findNode(TreeNode* node, const string& name) const {
        if (!node) return nullptr;
        if (node->name == name) return node;

        if (name < node->name) {
            return findNode(node->left, name);
        }
        else {
            return findNode(node->right, name);
        }
    }

    //  insert a new node
    TreeNode* insertNode(TreeNode* root, TreeNode* newNode) {
        if (!root) return newNode;

        if (newNode->name < root->name) {
            root->left = insertNode(root->left, newNode);
            root->left->parent = root;
        }
        else {
            root->right = insertNode(root->right, newNode);
            root->right->parent = root;
        }

        return root;
    }

public:
    FileSystemTree() {
        root = new TreeNode("Root");
        currentDir = root;
    }

    ~FileSystemTree() {
        deleteTree(root);
    }

    TreeNode* getCurrentDir() const {
        return currentDir;
    }

    string getCurrentDirectory() const {
        return currentDir->name;
    }

    bool rename_Directory(const string& oldName, const string& newName) {
        TreeNode* node = findNode(currentDir, oldName);
        if (node && !node->isFile) {
            node->name = newName;
            return true;
        }
        return false;
    }

    bool changeDirectory(const string& dirName) {
        if (dirName == "..") { // Move to parent directory
            if (currentDir->parent) {
                currentDir = currentDir->parent;
                return true;
            }
            cout << "Already at the root directory.\n";
            return false;
        }

        TreeNode* node = findNode(currentDir, dirName);
        if (node && !node->isFile) {
            currentDir = node;
            return true;
        }

        cout << "Directory not found: " << dirName << endl;
        return false;
    }

    bool makeDirectory(const string& dirName) {
        if (dirName.empty()) {
            cout << "Directory name cannot be empty.\n";
            return false;
        }

        if (findNode(currentDir, dirName)) {
            cout << "A directory with the name '" << dirName << "' already exists.\n";
            return false;
        }

        TreeNode* newDir = new TreeNode(dirName, false);
        currentDir = insertNode(currentDir, newDir);
        cout << "Directory '" << dirName << "' created successfully.\n";
        return true;
    }

    TreeNode* createFile(const string& fileName, const string& content = "") {
        if (findNode(currentDir, fileName)) {
            cout << "A file with the name '" << fileName << "' already exists in this directory.\n";
            return nullptr;
        }

        TreeNode* newFile = new TreeNode(fileName, true);
        newFile->content = content;
        currentDir = insertNode(currentDir, newFile);
        return newFile;
    }

    void listContents(TreeNode* node = nullptr) const {
        if (!node) node = currentDir;

        if (node->left) listContents(node->left);
        cout << (node->isFile ? "[File] " : "[Folder] ") << node->name << endl;
        if (node->right) listContents(node->right);
    }

    TreeNode* findFile(const string& fileName) const {
        return findNode(currentDir, fileName);
    }

    bool removeFile(const string& fileName) {
        TreeNode* fileToRemove = findFile(fileName);
        if (!fileToRemove || !fileToRemove->isFile)
            return false;

        TreeNode* parent = fileToRemove->parent;
        if (!parent)
            return false;

        if (parent->left == fileToRemove) {
            parent->left = nullptr;
        }
        else if (parent->right == fileToRemove) {
            parent->right = nullptr;
        }
        else {
            return false;
        }

        delete fileToRemove;
        return true;
    }
};

// Hash Table for File Metadata
class File_Meta_data {
public:
    string name;
    string type;
    size_t size;
    string owner;
    string creationDate;
    string lastModified;
    TreeNode* fileNode;
};

class HashTable {
private:
    static const int TABLE_SIZE = 100;

    struct HashEntry {
        string key;
        File_Meta_data* value;
        HashEntry* next;

        HashEntry(string k, File_Meta_data* v) : key(k), value(v), next(nullptr) {}
    };

    HashEntry** table;

    int hashFunction(const string& key) {
        int hash = 0;
        for (size_t i = 0; i < key.length(); i++) {
            hash = (hash * 31 + key[i]) % TABLE_SIZE;
        }
        return hash;
    }

public:
    HashTable() {
        table = new HashEntry * [TABLE_SIZE]();
    }

    ~HashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            HashEntry* entry = table[i];
            while (entry) {
                HashEntry* prev = entry;
                entry = entry->next;
                delete prev->value;
                delete prev;
            }
        }
        delete[] table;
    }

    void insert(const string& key, File_Meta_data* value) {
        int hash = hashFunction(key);
        HashEntry* prev = nullptr;
        HashEntry* entry = table[hash];

        while (entry && entry->key != key) {
            prev = entry;
            entry = entry->next;
        }

        if (!entry) {
            entry = new HashEntry(key, value);
            if (!prev) {
                table[hash] = entry;
            }
            else {
                prev->next = entry;
            }
        }
        else {
            delete entry->value;
            entry->value = value;
        }
    }

    File_Meta_data* search(const string& key) {
        int hash = hashFunction(key);
        HashEntry* entry = table[hash];
        while (entry) {
            if (entry->key == key) {
                return entry->value;
            }
            entry = entry->next;
        }
        return nullptr;
    }

    void remove(const string& key) {
        int hash = hashFunction(key);
        HashEntry* prev = nullptr;
        HashEntry* entry = table[hash];

        while (entry && entry->key != key) {
            prev = entry;
            entry = entry->next;
        }

        if (!entry) return;

        if (!prev) {
            table[hash] = entry->next;
        }
        else {
            prev->next = entry->next;
        }

        delete entry->value;
        delete entry;
    }
};

// Recycle Bin (Stack)
class Recycle_Bin {
private:
    struct BinNode {
        TreeNode* file;
        string deletionTime;
        BinNode* next;
    };

    BinNode* top;
public:
    Recycle_Bin() : top(nullptr) {}

    ~Recycle_Bin() {
        while (top) {
            BinNode* temp = top;
            top = top->next;
            delete temp;
        }
    }

    void push(TreeNode* file) {
        BinNode* newNode = new BinNode{ file, getCurrentTime(), top };
        top = newNode;
    }

    TreeNode* pop() {
        if (!top) return nullptr;
        BinNode* temp = top;
        TreeNode* file = temp->file;
        top = top->next;
        delete temp;
        return file;
    }

    bool isEmpty() const { return top == nullptr; }

    void display() const {
        if (isEmpty()) {
            cout << "Recycle Bin is empty.\n";
            return;
        }
        cout << "Recycle Bin contents:\n";
        BinNode* current = top;
        while (current) {
            cout << "- " << current->file->name << " (Deleted at: " << current->deletionTime << ")\n";
            current = current->next;
        }
    }
};

// Recent Files Queue
class Recent_Files_Queue {
private:
    struct QueueNode {
        TreeNode* file;
        QueueNode* next;
    };

    QueueNode* front;
    QueueNode* rear;
    int count;

public:
    Recent_Files_Queue() : front(nullptr), rear(nullptr), count(0) {}

    ~Recent_Files_Queue() {
        // Destructor to clean up all nodes
        while (front) {
            QueueNode* temp = front;
            front = front->next;
            delete temp;
        }
    }

    void enqueue(TreeNode* file) {
        // Create a new node for the recently accessed file
        QueueNode* newNode = new QueueNode{ file, nullptr };

        // If the queue is empty, new node becomes both front and rear
        if (!rear) {
            front = rear = newNode;
        }
        else {
            // Otherwise, add to the rear and update rear pointer
            rear->next = newNode;
            rear = newNode;
        }

        count++; // Increment the count of nodes in the queue
    }

    void dequeue() {
        // Remove the front node from the queue
        if (!front) return;
        QueueNode* temp = front;
        front = front->next;
        if (!front) rear = nullptr;
        delete temp;
        count--;
    }

    void display() const {
        if (!front) {
            cout << "No recent files\n";
            return;
        }

        cout << "Recently Accessed Files:\n";
        QueueNode* current = front;
        int index = 1;
        while (current) {
            if (current->file) {
                cout << index++ << ". " << current->file->name << endl;
            }
            current = current->next;
        }
    }
};
// User Graph (Linked List of Users)
class User_Graph {
public:
    struct UserNode {
        string userId;
        string password;
        string securityQuestion;
        string securityAnswer;
        string lastLogin;
        string lastLogout;

        struct SharedFile {
            string filename;
            string permission; // "view", "edit"
            UserNode* sharedWith;
            SharedFile* next;
        };

        SharedFile* sharedFiles;
        UserNode* next;
    };

    UserNode* users;

    User_Graph() : users(nullptr) {}

    ~User_Graph() {
        UserNode* current = users;
        while (current) {
            UserNode* nextUser = current->next;

            // Delete shared files list
            UserNode::SharedFile* sf = current->sharedFiles;
            while (sf) {
                UserNode::SharedFile* nextSF = sf->next;
                delete sf;
                sf = nextSF;
            }

            delete current;
            current = nextUser;
        }
    }

    UserNode* findUser(const string& userId) const {
        UserNode* current = users;
        while (current) {
            if (current->userId == userId) {
                return current;
            }
            current = current->next;
        }
        return nullptr;
    }

    bool addUser(const string& userId, const string& password, const string& question, const string& answer) {
        if (findUser(userId)) {
            return false;
        }

        UserNode* newUser = new UserNode{ userId, password, question, answer, "", "", nullptr, users };
        users = newUser;
        return true;
    }

    UserNode* authenticate(const string& userId, const string& password) {
        UserNode* user = findUser(userId);
        if (user && user->password == password) {
            user->lastLogin = getCurrentTime();
            return user;
        }
        return nullptr;
    }

    bool recoverPassword(const string& userId) {
        UserNode* user = findUser(userId);
        if (!user) {
            cout << "User ID not found.\n";
            return false;
        }

        cout << "Security Question: " << user->securityQuestion << endl;
        cout << "Enter your answer: ";
        string answer;
        cin.ignore();
        getline(cin, answer);

        if (answer == user->securityAnswer) {
            cout << "Your password is: " << user->password << endl;
            return true;
        }
        else {
            cout << "Incorrect answer. Cannot recover password.\n";
            return false;
        }
    }

    void logout(UserNode* user) {
        if (user) {
            user->lastLogout = getCurrentTime();
        }
    }

    bool shareFile(UserNode* owner, const string& targetUserId, const string& filename, const string& permission) {
        UserNode* target = findUser(targetUserId);
        if (!target)
            return false;

        UserNode::SharedFile* newShare = new UserNode::SharedFile{
            filename, permission, target, owner->sharedFiles };
        owner->sharedFiles = newShare;
        return true;
    }

    void displaySharedFiles(UserNode* user) const {
        if (!user || !user->sharedFiles) {
            cout << "No files shared\n";
            return;
        }

        cout << "Files shared by " << user->userId << ":\n";
        UserNode::SharedFile* sf = user->sharedFiles;
        while (sf) {
            cout << "- " << sf->filename << " with " << sf->sharedWith->userId
                << " (" << sf->permission << " access)\n";
            sf = sf->next;
        }
    }
};

// File Versioning System
class File_Version_List {
private:
    struct VersionNode {
        int versionNumber;
        string content;
        string modificationTime;
        VersionNode* prev;
        VersionNode* next;
    };

    VersionNode* head;
    VersionNode* tail;
    int currentVersion;

public:
    File_Version_List() : head(nullptr), tail(nullptr), currentVersion(0) {}

    ~File_Version_List() {
        VersionNode* current = head;
        while (current) {
            VersionNode* next = current->next;
            delete current;
            current = next;
        }
    }

    void addVersion(const string& content) {
        currentVersion++;
        VersionNode* newNode = new VersionNode{
            currentVersion,
            content,
            getCurrentTime(),
            tail,
            nullptr
        };

        if (tail) {
            tail->next = newNode;
        }
        else {
            head = newNode;
        }
        tail = newNode;
    }

    string getVersion(int versionNumber) const {
        VersionNode* current = head;
        while (current) {
            if (current->versionNumber == versionNumber) {
                return current->content;
            }
            current = current->next;
        }
        return "";
    }

    void display_Versions() const {
        if (!head) {
            cout << "No versions available\n";
            return;
        }

        cout << "File Version History:\n";
        VersionNode* current = head;
        while (current) {
            cout << "Version " << current->versionNumber << " ("
                << current->modificationTime << ")\n";
            current = current->next;
        }
    }
};

// The Google Drive System
class Google_Drive_System {
private:
    FileSystemTree fileSystem;
    HashTable fileMetadata;
    Recycle_Bin recycleBin;
    Recent_Files_Queue recentFiles;
    User_Graph userGraph;
    User_Graph::UserNode* currentUser;

public:
    Google_Drive_System() : currentUser(nullptr) {
        // Initialize with admin user
        userGraph.addUser("admin", "password", "Favorite color?", "blue");
    }

    ~Google_Drive_System() {
        if (currentUser) {
            userGraph.logout(currentUser);
        }
    }

    void display_Main_Menu() {
        cout << "====================================================\n";
        cout << "\n   WELCOME TO GOOGLE DRIVE MANAGEMENT SYSTEM\n";
        cout << "  1. Login\n";
        cout << "  2. Browse Files\n";
        cout << "  3. Share File\n";
        cout << "  4. View Version History\n";
        cout << "  5. Access Recycle Bin\n";
        cout << "  6. View Recent Files\n";
        cout << "  7. Add New User\n";
        cout << "  8. Recover Password\n";
        cout << "  9. Logout\n";
        cout << "  10. Compression algorithm\n";
        cout << "    11. Exit\n";
        cout << "Enter your choice (1-10): ";
    }

    void run() {
        while (true) {
            srand(time(0));
            int background = rand() % 8;
            int text = rand() % 16;

            // Ensure background and text colors are not the same
            while (background == text) {
                text = rand() % 16;
            }

            char colorCode[3];
            snprintf(colorCode, sizeof(colorCode), "%X%X", background, text);

            string command = string("color ") + colorCode;
            system(command.c_str());

            // Display main menu
            display_Main_Menu();

            int choice;
            cin >> choice;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a number between 1 and 10.\n";
                continue;
            }            switch (choice) {
            case 1: log_in(); break;
            case 2: browse_Files(); break;
            case 3: share_File(); break;
            case 4: view_Version_History(); break;
            case 5: accessRecycleBin(); break;
            case 6: recentFiles.display(); break;
            case 7: addUser(); break;
            case 8: recoverPassword(); break;
            case 9: log_out(); break;
            case 10: compressionAlgorithm(); break;
            case 11: return;
            default:
                cout << "Invalid choice. Please enter a number between 1 and 10.\n";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
    }

    void log_in() {
        if (currentUser) {
            cout << "Already logged in as " << currentUser->userId << endl;
            return;
        }

        string userId, password;
        cout << "Enter user ID: ";
        cin >> userId;
        cout << "Enter password: ";
        cin >> password;

        currentUser = userGraph.authenticate(userId, password);
        if (currentUser) {
            cout << "Login successful! Welcome, " << userId << endl;
            cout << "Current Time: " << getCurrentTime() << endl;
        }
        else {
            cout << "Login failed. Invalid credentials.\n";
        }
    }

    void log_out() {
        if (currentUser) {
            userGraph.logout(currentUser);
            cout << "Logged out successfully. Goodbye, " << currentUser->userId << endl;
            cout << "Current Time: " << getCurrentTime() << endl;
            currentUser = nullptr;
        }
        else {
            cout << "No user is currently logged in.\n";
        }
    }

    void compressionAlgorithm() {
        cout << "Applying file compression (RLE)...\n";
        string content;
        cout << "Enter file content to compress: ";
        cin.ignore();
        getline(cin, content);

        // Implement Run-Length Encoding (RLE)
        string compressed = "";
        int count = 1;
        for (size_t i = 1; i < content.length(); i++) {
            if (content[i] == content[i - 1]) {
                count++;
            }
            else {
                compressed += to_string(count) + content[i - 1];
                count = 1;
            }
        }
        compressed += to_string(count) + content[content.length() - 1];
        cout << "Compressed content: " << compressed << endl;
    }

    void browse_Files() {
        if (!currentUser) {
            cout << "\\\\\\\\ Please login first ////////\n";
            return;
        }

        while (true) {
            try {
                fileSystem.listContents();

                cout << "\n1. Change directory\n";
                cout << "2. Create directory\n";
                cout << "3. Upload file\n";
                cout << "4. Download file\n";
                cout << "5. Edit/Update file\n";
                cout << "6. Delete file\n";
                cout << "7. Back to main menu\n";

                cout << "Enter choice: ";
                string input;
                cin >> input;

                if (input.length() != 1 || input[0] < '1' || input[0] > '7') {
                    throw invalid_argument("Invalid input! Please enter a number from 1 to 7.");
                }

                int choice = stoi(input);

                if (choice == 1) {  // Change directory
                    string dirName;
                    cout << "Enter directory name (or '..' for parent): ";
                    cin >> dirName;

                    if (fileSystem.changeDirectory(dirName)) {
                        cout << "Changed to directory: " << dirName << endl;
                    }
                    else {
                        cout << "Directory not found.\n";
                    }
                }
                else if (choice == 2) {  // Create directory
                    string dirName;
                    cout << "Enter new directory name: ";
                    cin >> dirName;

                    if (fileSystem.makeDirectory(dirName)) {
                        cout << "Directory '" << dirName << "' created successfully.\n";
                    }
                    else {
                        cout << "Failed to create directory.\n";
                    }
                }
                else if (choice == 3) {  // Upload file
                    string fileName, content;
                    cout << "Enter file name: ";
                    cin >> fileName;
                    cout << "Enter file content: ";
                    cin.ignore();
                    getline(cin, content);

                    if (fileSystem.findFile(fileName)) {
                        cout << "File '" << fileName << "' already exists in the directory.\n";
                        continue;
                    }

                    TreeNode* newFile = fileSystem.createFile(fileName, content);
                    if (!newFile) {
                        cout << "Failed to create the file. Please try again.\n";
                        continue;
                    }

                    // Add metadata
                    File_Meta_data* metaData = new File_Meta_data();
                    metaData->name = fileName;
                    metaData->type = "txt";
                    metaData->size = content.size();
                    metaData->owner = currentUser ? currentUser->userId : "Unknown";
                    metaData->creationDate = getCurrentTime();
                    metaData->lastModified = getCurrentTime();
                    metaData->fileNode = newFile;

                    fileMetadata.insert(fileName, metaData);
                    cout << "File '" << fileName << "' uploaded successfully.\n";

                    // Add to Recent Files
                    recentFiles.enqueue(newFile);
                }
                else if (choice == 4) {  // Download file
                    string fileName;
                    cout << "Enter file name to download: ";
                    cin >> fileName;

                    File_Meta_data* meta = fileMetadata.search(fileName);
                    if (meta && meta->fileNode) {
                        TreeNode* file = meta->fileNode;
                        cout << "\nFile Name: " << meta->name << endl;
                        cout << "Type: " << meta->type << endl;
                        cout << "Size: " << meta->size << " bytes" << endl;
                        cout << "Owner: " << meta->owner << endl;
                        cout << "Created: " << meta->creationDate << endl;
                        cout << "Last Modified: " << meta->lastModified << endl;
                        cout << "Content:\n" << file->content << endl;
                        cout << "File downloaded successfully!\n";

                        // Add to Recent Files
                        recentFiles.enqueue(file);
                    }
                    else {
                        cout << "File not found.\n";
                    }
                }
                else if (choice == 5) {  // Edit/Update file
                    string fileName, newContent;
                    cout << "Enter the name of the file to edit: ";
                    cin >> fileName;

                    File_Meta_data* meta = fileMetadata.search(fileName);
                    if (!meta || !meta->fileNode) {
                        cout << "File not found.\n";
                        continue;
                    }

                    if (meta->owner != currentUser->userId) {
                        cout << "Error: You don't have permission to edit this file.\n";
                        continue;
                    }

                    cout << "Enter new content for the file:\n";
                    cin.ignore();
                    getline(cin, newContent);

                    TreeNode* file = meta->fileNode;
                    file->content = newContent;  // Update file content
                    meta->size = newContent.size();  // Update metadata size
                    meta->lastModified = getCurrentTime();  // Update last modified timestamp

                    cout << "File '" << fileName << "' updated successfully.\n";

                    // Add to Recent Files
                    recentFiles.enqueue(file);
                }
                else if (choice == 6) {  // Delete file
                    string fileName;
                    cout << "Enter the name of the file to delete: ";
                    cin >> fileName;

                    TreeNode* fileToDelete = fileSystem.findFile(fileName);
                    if (!fileToDelete) {
                        cout << "File not found.\n";
                        continue;
                    }

                    File_Meta_data* meta = fileMetadata.search(fileName);
                    if (!meta) {
                        cout << "Metadata for the file not found. Cannot delete.\n";
                        continue;
                    }

                    if (meta->owner != currentUser->userId) {
                        cout << "Error: You don't have permission to delete this file.\n";
                        continue;
                    }

                    if (fileSystem.removeFile(fileName)) {
                        recycleBin.push(fileToDelete);
                        fileMetadata.remove(fileName);
                        cout << "File '" << fileName << "' has been deleted and moved to the Recycle Bin.\n";

                        // Add to Recent Files (optional, for deleted files)
                        recentFiles.enqueue(fileToDelete);
                    }
                    else {
                        cout << "Failed to delete the file.\n";
                    }
                }
                else if (choice == 7) {  // Exit
                    cout << "Returning to the main menu...\n";
                    break;
                }
            }
            catch (const invalid_argument& e) {
                cout << e.what() << "\nPlease enter a valid number from 1 to 7.\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            catch (...) {
                cout << "An unexpected error occurred. Please try again.\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
    }

    void share_File() {
        if (!currentUser) {
            cout << "\\\\\\\\ Please login first ////////\n";
            return;
        }
        string fileName, targetUser, permission;
        cout << "Enter file name to share: ";
        cin >> fileName;
        cout << "Enter user to share with: ";
        cin >> targetUser;
        cout << "Enter permission (view/edit): ";
        cin >> permission;

        if (fileSystem.findFile(fileName)) {
            if (userGraph.shareFile(currentUser, targetUser, fileName, permission)) {
                cout << "File shared successfully with " << targetUser << endl;
            }
            else {
                cout << "Failed to share file. User not found.\n";
            }
        }
        else {
            cout << "File not found\n";
        }
    }

    void view_Version_History() {
        if (!currentUser) {
            cout << "\\\\\\\\ Please login first ////////\n";
            return;
        }
        string fileName;
        cout << "Enter file name: ";
        cin >> fileName;

        File_Version_List versions;
        TreeNode* file = fileSystem.findFile(fileName);
        if (file) {
            versions.addVersion(file->content);
            versions.addVersion("Previous version content");
            versions.addVersion("Original content");

            versions.display_Versions();

            cout << "\nEnter version number to view (0 to cancel): ";
            int version;
            cin >> version;

            if (version > 0) {
                string content = versions.getVersion(version);
                if (!content.empty()) {
                    cout << "\nVersion " << version << " content:\n";
                    cout << content << endl;
                }
                else {
                    cout << "Invalid version number\n";
                }
            }
        }
        else {
            cout << "File not found\n";
        }
    }

    void accessRecycleBin() {
        if (!currentUser) {
            cout << "\\\\\\\\ Please login first ////////\n";
            return;
        }

        while (true) {
            cout << "\n=====================\n";
            recycleBin.display();

            cout << "\nSelect an option:\n";
            cout << "1. Restore a file\n";
            cout << "2. Empty recycle bin\n";
            cout << "3. Back to main menu\n";

            int choice;
            cin >> choice;

            if (choice == 1) {
                TreeNode* restoredFile = recycleBin.pop();
                if (restoredFile) {
                    TreeNode* parent = fileSystem.getCurrentDir();
                    restoredFile->parent = parent;

                    TreeNode* newNode = fileSystem.createFile(restoredFile->name, restoredFile->content);
                    if (!newNode) {
                        cout << "Failed to restore file.\n";
                        return;
                    }

                    File_Meta_data* meta = new File_Meta_data();
                    meta->name = restoredFile->name;
                    meta->type = "txt";
                    meta->size = restoredFile->content.size();
                    meta->owner = currentUser->userId;
                    meta->creationDate = getCurrentTime();
                    meta->lastModified = getCurrentTime();
                    meta->fileNode = newNode;
                    fileMetadata.insert(restoredFile->name, meta);

                    cout << "File '" << restoredFile->name << "' has been restored.\n";
                }
                else {
                    cout << "Recycle Bin is empty.\n";
                }
            }
            else if (choice == 2) {
                while (!recycleBin.isEmpty()) {
                    TreeNode* deletedFile = recycleBin.pop();
                    delete deletedFile;
                }
                cout << "Recycle Bin emptied.\n";
            }
            else if (choice == 3) {
                cout << "Returning to main menu.\n";
                break;
            }
            else {
                cout << "Invalid choice. Please try again.\n";
            }
        }
    }

    void addUser() {
        if (!currentUser) {
            cout << "\\\\\\\\ Please login first ////////\n";
            return;
        }

        if (currentUser->userId != "admin") {
            cout << "Only the admin can add new users.\n";
            return;
        }

        string userId, password, question, answer;
        cout << "Enter new user ID: ";
        cin >> userId;
        cout << "Enter new password: ";
        cin >> password;
        cout << "Enter security question: ";
        cin.ignore();
        getline(cin, question);
        cout << "Enter answer to the security question: ";
        getline(cin, answer);

        if (userGraph.addUser(userId, password, question, answer)) {
            cout << "User '" << userId << "' added successfully!\n";
        }
        else {
            cout << "User ID '" << userId << "' already exists. Try a different ID.\n";
        }
    }

    void recoverPassword() {
        string userId;
        cout << "Enter your user ID: ";
        cin >> userId;

        if (!userGraph.recoverPassword(userId)) {
            cout << "Password recovery failed.\n";
        }
    }
};

int main() {
    Google_Drive_System driveSystem;
    driveSystem.run();
    system("pause");
    return 0;
}
