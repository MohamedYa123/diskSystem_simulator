#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <list>
#include<sstream>  
#include<vector>
using namespace std;
#define DISK_SIZE 256
#include <stdio.h>
// ============================================================================
void decToBinary(int n, char& c)
{
    // array to store binary number
    int binaryNum[8];

    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--)
    {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }
}



// ============================================================================

class FsFile {


public:
    int file_size;
    int block_in_use;
    int index_block;
    int block_size;
    FsFile(int _block_size) {
        file_size = 0;
        block_in_use = 0;
        block_size = _block_size;
        index_block = -1;
    }

    int getfile_size() {
        return file_size;
    }

};

// ============================================================================

class FileDescriptor {
    FsFile* fs_file;
    string file_name;

    int blockindex = -1;

public:
    bool open;
    bool inUse;
    FileDescriptor(string FileName, FsFile* fsi) {
        file_name = FileName;
        fs_file = fsi;
        inUse = true;
    }
    FsFile* getfile() {
        return fs_file;
    }
    int getindexblock() {
        return fs_file->index_block;
    }
    string getFileName() {
        return file_name;
    }
    void openme() {
        open = true;
        inUse = true;
    }
    void closeme() {
        open = false;
        inUse = false;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

// ============================================================================
class fsDisk
{
    FILE* sim_disk_fd;
    bool is_formated = false;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize = 4;
    int* BitVector;
    int numblocks = 0;
    string disk[DISK_SIZE] = {};
    // int bksize = 4;
    vector<FileDescriptor*> maindir;
    vector< FileDescriptor*>openFileDescriptors;
public:
    fsDisk() {
        sim_disk_fd = fopen(DISK_SIM_FILE, "r+");
        //assert(sim_disk_fd);
        /*
        for (int i = 0; i < DISK_SIZE; i++) {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fwrite("\0", 1, 1, sim_disk_fd);
            assert(ret_val == 1);
        }

        fflush(sim_disk_fd);
        is_formated = false;
        */

    }
    void print_bits() {
        cout << "\n";
        for (int i = 0; i < numblocks; i++) {
            cout << BitVector[i] << ",";
        }
        cout << "\n";
    }
    void listAll() {
        int i = 0;

        for (int i = 0; i < maindir.size(); i++) {
            cout << "index: " << i << ": FileName: " << maindir[i]->getFileName() << " , isInUse: " << maindir[i]->inUse << endl;
            //  i++;
        }

        string bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++)
        {
            cout << "(";
            // int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            // ret_val = fread(&bufy, 1, 1, sim_disk_fd);
            bufy = disk[i];
            cout << bufy;
            cout << ")";
        }
        cout << "'" << endl;
    }
    // ------------------------------------------------------------------------
    void fsFormat(int blockSize = 4) {

        BitVectorSize = blockSize;
        for (int i = 0; i < sizeof(disk) / sizeof(disk[0]); i++)
        {
            disk[i] = "\0";
        }
        maindir.clear();
        openFileDescriptors.clear();
        is_formated = true;
        int ihg = DISK_SIZE / BitVectorSize;
        numblocks = ihg;
        BitVector = new int[ihg];
        for (int iu = 0; iu < ihg; iu++) {
            BitVector[iu] = 0;    // Initialize all elements to zero.
        }
        cout << "Format disk : number of blocks : " << 256 / blockSize << "\n";
    }
    int check(FileDescriptor* fsf, int len) {
        if (!is_formated) {
            cout << "disk isn't intialised\n";
            return -1;
        }
        if (!fsf->open) {
            cout << "the file isn't opened\n";
            return -1;
        }
        //check for enough space
        int numfree = 0;
        int mainlen = len;
        for (int i = 0; i < numblocks; i++) {
            if (BitVector[i] == 0) {
                len--;
                numfree++;
            }
        }
        // if (len > 0) {
             //check in the file for enough space //
        FsFile* fs = fsf->getfile();
        int indexblock = fs->index_block;
        int indx = indexblock * BitVectorSize;
        if (disk[indx] == "\0") {
            if (getfreeblock() == -1 || mainlen / BitVectorSize > numfree) {
                cout << "no enough space in disk !\n";
                return -1;
            }
            if (mainlen / BitVectorSize > BitVectorSize) {
                cout << "no enough space in the file !\n";
                return -1;
            }
        }
        else {
            int icheck = 0;
            for (int i = 0; i < BitVectorSize - 1; i++) {
                if (disk[indx + i + 1] == "\0") {
                    icheck = i;
                    break;
                }
            }
            //stoi
            string s = disk[indx + icheck];
            int blockindex = stoi(s) * BitVectorSize;
            int spacefound = 0;
            for (int i3 = 0; i3 < BitVectorSize; i3++) {
                if (disk[blockindex + i3] == "\0") {
                    spacefound++;
                }
            }
            int nf = numfree;
            for (icheck = icheck + 1; icheck < BitVectorSize; icheck++) {
                if (nf > 0) {
                    spacefound += BitVectorSize;
                    nf--;
                }
            }
            if (mainlen > spacefound) {
                cout << "no enough space in the file !\n";
                return -1;
            }
            else if (spacefound < mainlen) {
                cout << "no enough space in disk !\n";
                return -1;
            }
        }
        // cout << "no enough space !\n";
       //  return -1;
        return 0;
        //  }
    }
    // ------------------------------------------------------------------------
    int CreateFile(string fileName) {
        FsFile* f = new FsFile(BitVectorSize);
        int idbk = getfreeblock();
        if (idbk == -1) {
            return -1;
        }
        f->index_block = idbk / BitVectorSize;
        FileDescriptor* fd = new FileDescriptor(fileName, f);
        fd->openme();
        openFileDescriptors.push_back(fd);
        if (check(fd, 0) == -1) {
            return -1;
        }
        maindir.push_back(fd);
        //  cout << "Create file "<<fileName <<" with file descriptor"<<"\n";
        return openFileDescriptors.size() - 1;
    }

    // ------------------------------------------------------------------------
    int OpenFile(string fileName) {
        FileDescriptor* flmain = maindir[0];// get(maindir, 0);;
        for (int i = 0; i < maindir.size(); i++)
        {
            FileDescriptor* fl = maindir[i];// get(maindir, i);

            if (fl->getFileName() == fileName) {
                flmain = fl;//  fl->openme();
            }
            if (!fl->open) {
                //     cout << "\nthere is an open file : " << fl->getFileName() << " at descriptor : #" << i;
               //      return -1;
                openFileDescriptors.push_back(fl);
            }
        }
        flmain->openme();
        return 0;
    }

    // ------------------------------------------------------------------------
    string CloseFile(int fd) {
        FileDescriptor* fl = maindir[fd];// get(maindir, fd);
        if (fl->open) {
            openFileDescriptors.erase(std::next(openFileDescriptors.begin(), fd));
            fl->closeme();
        }
        else {

            // cout << "\nthe file isn't opened !";
            // return "-1";
        }
        return fl->getFileName();
    }
    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char* buf, int len) {

        FileDescriptor* fdd = openFileDescriptors[fd];// get(maindir, fd);
        if (!fdd->open) {
            cout << "\nThe file isn't oppened !\n";
            return -2;
        }
        int j = strlen(buf);// sizeof(buf);// / sizeof(buf[0]);
        if (check(fdd, j) == -1) {//sizeof(buf) / sizeof(buf[0])
            return -1;
        }
        FsFile* f;
        f = fdd->getfile();
        if (write_at_block(f->index_block, buf) != -1) {
            // if (u != -1) {
            assure_write(buf);
            // }
            return 0;
        }
        else {
            return -1;
        }
    }

    // ------------------------------------------------------------------------
    int DelFile(string FileName) {
        for (int ii = 0; ii < maindir.size(); ii++)
        {
            FileDescriptor* fl = maindir[ii];// get(maindir, ii);
            if (fl->getFileName() == FileName) {
                if (fl->open) {
                    cout << "\nPlease close the file before you delete it .\n";
                    return -1;
                }
                for (int i = 0; i < sizeof(disk) / sizeof(disk[0]); i++)
                {
                    // if (fl->getfile()->index_block) {
                    for (int i = fl->getfile()->index_block * BitVectorSize; i < fl->getfile()->index_block * BitVectorSize + BitVectorSize; i++) {
                        if (disk[i] != "\0") {
                            int n = stoi(disk[i]) * BitVectorSize;
                            for (int i2 = n; i2 < n + BitVectorSize; i2++) {
                                disk[i2] = "\0";
                                BitVector[i2 / BitVectorSize] = 0;
                            }
                        }
                        disk[i] = "\0";
                        BitVector[i / BitVectorSize] = 0;
                    }
                }
                maindir.erase(std::next(maindir.begin(), ii));//.remove(fl);
                return ii;
            }
        }

        return 0;
    }
    // ------------------------------------------------------------------------
    string ReadFromFile(int fd, char* buf, int len) {

        FileDescriptor* fl = openFileDescriptors[fd];// get(maindir, fd);
        if (!fl->open) {
            cout << "\nThe file isn't oppened !\n";
            return "-1";
        }
        // for (int ii = 0; ii < sizeof(disk) / sizeof(disk[0]); ii++)
        {
            // if (fl->getfile()->index_block) {
            for (int i = fl->getfile()->index_block * BitVectorSize; i < fl->getfile()->index_block * BitVectorSize + BitVectorSize; i++) {
                if (disk[i] != "\0") {
                    int n = stoi(disk[i]) * BitVectorSize;
                    for (int i2 = n; i2 < n + BitVectorSize; i2++) {
                        cout << disk[i2];// = "\0";
                        len--;
                        if (len <= 0) {
                            cout << "\n";
                            return fl->getFileName();
                        }
                    }
                }

                //  disk[i] = "\0";
            }
        }

        cout << "\n";
        string ans = fl->getFileName();
        return ans;
    }

    int assure_write(char* listchars) {
        int i = 0;
        for (int i2 = 0; i2 < sizeof(disk) / sizeof(disk[0]); i2++) {
            if (disk[i2] == "k2") {
                disk[i2] = listchars[i];
                i++;
            }
        }
        return 0;
    }
    int cancel_write() {
        for (int i2 = 0; i2 < sizeof(disk) / sizeof(disk[0]); i2++) {
            if (disk[i2] == "k2") {
                disk[i2] = "\0";
            }
        }
        return 0;
    }
private:
    int getfreeblock() {
        for (int i = 0; i < sizeof(disk) / sizeof(disk[0]); i += BitVectorSize)
        {
            int l = 0;
            for (int i2 = i; i2 < i + BitVectorSize; i2++) {
                if (disk[i] != "\0") {
                    l = 1;
                }
            }
            if (l == 0) {
                return i;
            }
        }
        return -1;
    }
    int nex = 0;
    int write_at_block(int indexblock, char* listchars) {
        char jh = listchars[nex];
        if (jh == '\0') {
            nex = 0;
            return 0;
        }
        for (int i2 = indexblock * BitVectorSize; i2 < indexblock * BitVectorSize + BitVectorSize; i2++) {
            if (disk[i2] == "\0") {
                int i = getfreeblock();
                if (i == -1) {
                    return -1;
                }
                disk[i2] = conv(i);// static_cast<char>(i);
                i = getfreeblock();
                disk[i2] = conv(i / BitVectorSize);
                BitVector[i2 / BitVectorSize] = 1;
                i2--;
            }
            else {
                int n = stoi(disk[i2]) * BitVectorSize;
                for (int i = n; i < n + BitVectorSize; i++)
                {
                    // int index= static_cast<int>(disk[i]);
                    if (disk[i] == "\0")
                    {
                        BitVector[i / BitVectorSize] = 1;
                        // char jh = listchars[2];
                        disk[i] = "k2";// listchars[nex];
                        nex++;
                        return write_at_block(indexblock, listchars);
                    }
                }

            }

        }
        return -1;
    }

    string conv(int number) {
        stringstream ss;
        ss << number;
        string s;
        ss >> s;
        return s;// num_char[0];
    }
    FileDescriptor* get(list<FileDescriptor*> _list, int _i) {
        list<FileDescriptor*>::iterator it = _list.begin();
        for (int i = 0; i < _i; i++) {
            ++it;
        }
        return *it;
    }

};


int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;
    int u;
    fsDisk* fs = new fsDisk();
    int cmd_;
    while (1) {
        cin >> cmd_;
        switch (cmd_)
        {
        case 0:   // exit
           // delete fs;
            exit(0);
            break;

        case 1:  // list-file
            fs->listAll();
            break;

        case 2:    // format
            cin >> blockSize;
            fs->fsFormat(blockSize);
            break;

        case 3:    // create-file
            cin >> fileName;
            _fd = fs->CreateFile(fileName);
            cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;
        case 4:  // open-file
            cin >> fileName;
            _fd = fs->OpenFile(fileName);
            if (_fd != -1) {
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            }
            break;

        case 5:  // close-file
            cin >> _fd;
            fileName = fs->CloseFile(_fd);
            //   if (fileName != "n-1") {
            cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            //   }
            break;

        case 6:   // write-file
            cin >> _fd;
            cin >> str_to_write;
            u = fs->WriteToFile(_fd, str_to_write, strlen(str_to_write));

            //  else {
              //    fs->cancel_write();
             //     cout << "\n no enough space\n";
             // }
            break;

        case 7:    // read-file
            cin >> _fd;
            cin >> size_to_read;
            fileName = fs->ReadFromFile(_fd, str_to_read, size_to_read);
            //   if (fileName != "n-1") {
            cout << "ReadFromFile: " << fileName << endl;
            //    }
            break;

        case 8:   // delete file 
            cin >> fileName;
            _fd = fs->DelFile(fileName);
            //  if (_fd != -1) {
            cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            //  }
            break;
            //   case 9:
                   //print bitvector
              //     fs->print_bits();
        default:
            cout << "\n";
            break;
        }
    }

}
