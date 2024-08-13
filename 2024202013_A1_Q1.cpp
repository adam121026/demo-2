#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <cstring>
#include<iostream>
#include <string>
#include <vector> 
#include <cstdlib>
using namespace std;


//PATHS//
string DIR_PATH = "./Assignment1/";
//Error Msgs//
string NO_INDEX = "provide the start and end indices";
string FORMAT = "provide the argument in proper fromat of ./a.out <input_file> <flag 0 or 1> <start_index> <end_index>";
string WRONG_INDEX_1 = "start index should be more than than end index";
string WRONG_INDEX_2 = "indices cannot be negative";
string WRITE = "unable to write the file";
string READ = "unable to read the file";
string DIR = "unable to create the directory";
string INPUT_PATH = "Input file doesnt exist in the give Path: ";

void displayError(string msg) {
    errno = 1;
    perror(msg.c_str());
    errno = 0;
     _exit(EXIT_FAILURE);
}


string getFileName(string path) {
    ssize_t lastSlash = path.find_last_of('/');
    if (lastSlash == -1) return path;
    else return path.substr(lastSlash + 1, path.size());

}

void  create_directory() {
    int err = mkdir(DIR_PATH.c_str(), 0755);
    if (err == -1 && errno != EEXIST) {
        displayError(DIR);
    }
}

int create_file(const char* path) {
    return open(path,  O_RDWR | O_CREAT | O_TRUNC, 0666);
}

int open_file(string path) {
    int fileNo =  open(path.c_str(), O_RDONLY);
    if (fileNo == -1) {
        displayError(INPUT_PATH + path);
    }
    return fileNo;
}

off_t getfilesize(int fd) {
    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET); 
    return size;
}

ssize_t convert_to_integer(char* s) {
    ssize_t n = strlen(s);
    ssize_t integer = 0;
    ssize_t power_10 = 1;
    for(int i = n-1; i>=0; i--) {
        integer += (s[i] - '0') * (power_10);
        power_10 *= 10;
    }
    return integer;
}

void displayProgress(ssize_t data_written, ssize_t fileSize, string& percentage ) {
    int  progress = (data_written * 100) / fileSize;
    percentage   = to_string(progress) + "%";
}


void reverse_chunks(char* buffer, ssize_t chunk_size) {
    int i = 0;
    int j = chunk_size-1;
    int n = i - (i-j)/2;
    while(i <= n) {
        char temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
        i++;
        j--;
    }
}

ssize_t write_to_file_with_given_size(int fd, int writefile_fd ,ssize_t offSet, char* buffer, ssize_t bytes_to_read, int rev) {
        lseek(fd, offSet, SEEK_SET);
         ssize_t curr_read = read(fd, buffer, bytes_to_read);
         if (curr_read == -1) {
            displayError(READ);
         }
         if (rev) reverse_chunks(buffer, curr_read);
         int write_fd =  write(writefile_fd, buffer, curr_read);
         if (write_fd == -1) {
            displayError(WRITE);
         }
         return write_fd;

}


ssize_t traversing_the_file(int fd, int writefile_fd, ssize_t fileSize, ssize_t end, ssize_t bufferSize, int rev, char* buffer, int total_bytes_written) {
        ssize_t offSet = fileSize;
        ssize_t bytes_to_read = bufferSize;
        string percentage;
        int rem_chunk = (fileSize - end) % bufferSize;
        int err;

        for (ssize_t offSet = fileSize-bufferSize; offSet>=end; offSet-=bytes_to_read) {
            ssize_t data_written = write_to_file_with_given_size(fd, writefile_fd, offSet, buffer, bytes_to_read, rev);
                total_bytes_written += data_written;
                displayProgress(total_bytes_written, fileSize, percentage);
                err = write(1, "\r\x1b[2K", 5);
                if (err == -1) {
                    displayError(WRITE);
                }
                err = write(1, percentage.c_str(), percentage.size());
                if (err == -1) {
                    displayError(WRITE);
                }
        }

        // if rem_chunk exists read the last chunk and write the file//
        if (rem_chunk) {
            ssize_t data_written = write_to_file_with_given_size(fd, writefile_fd, end, buffer, rem_chunk, 1);
            total_bytes_written += data_written;
            displayProgress(total_bytes_written, fileSize, percentage);
            write(1, "\r\x1b[2K", 5);
            write(1, percentage.c_str(), percentage.size());
        }
    return total_bytes_written;
}

int main( int argc, char *argv[]) {
    string input_file;
    ssize_t curr_chunk;
    ssize_t start = 0;
    ssize_t end = 0;


    //bytes//
    ssize_t bufferSize = 1024 * 1024;
    char buffer[bufferSize];
    off_t fileSize;

    if (argv[2] == "1" && argc < 5) {
        displayError(NO_INDEX);
    } else if (argc < 3) {
        displayError(FORMAT);
    }

    string input_file_path = argv[1];
    input_file = getFileName(input_file_path);


    int flag = convert_to_integer(argv[2]);

    if (flag) {
        if (argc < 5) displayError(FORMAT);
        start = convert_to_integer(argv[3]);
        end = convert_to_integer(argv[4]);
    }

    if (start < 0 || end< 0) {
        displayError(WRONG_INDEX_2);
    }

    if (start > end) {
        displayError(WRONG_INDEX_1);
    }

    //input file//
    int fd = open_file(input_file_path);
    fileSize = getfilesize(fd);

    //file dir//
    create_directory();

    //output_file//
    string output_file = DIR_PATH + to_string(flag) + "_" + input_file;
    int writefile_fd = create_file(output_file.c_str());
    size_t total = 0;
    ssize_t total_bytes_written = 0;

    if (!flag) {
        //traversing file and revrsing the chunks//
    total_bytes_written =   traversing_the_file(fd, writefile_fd, fileSize, 0, bufferSize, 1, buffer, 0);

        }
    else {
        //left half reversal//
        total_bytes_written =  traversing_the_file(fd, writefile_fd, start, 0, bufferSize, 1, buffer, total_bytes_written);

//--------------------------------------------------------------------------------------------------------------------------------//
        //middle chunk reversal//
            ssize_t midChunk = abs(end - start)+1;
            ssize_t rem_chunk = midChunk % bufferSize;
            ssize_t bytes_to_read = bufferSize;
            string percentage;  
            ssize_t offSet;                                                                                                                                                                    
            for (offSet = start; offSet<=end-bytes_to_read + 1; offSet+=bytes_to_read) {
                ssize_t data_written = write_to_file_with_given_size(fd, writefile_fd, offSet, buffer, bytes_to_read, 0);
                total_bytes_written += data_written;
                displayProgress(total_bytes_written, fileSize, percentage);
                write(1, "\r\x1b[2K", 5);
                write(1, percentage.c_str(), percentage.size());
        }
            if (rem_chunk) {
            ssize_t data_written = write_to_file_with_given_size(fd, writefile_fd, offSet, buffer, rem_chunk, 0);
            total_bytes_written += data_written;
            displayProgress(total_bytes_written, fileSize, percentage);
            write(1, "\r\x1b[2K", 5);
            write(1, percentage.c_str(), percentage.size());

        }
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
    //right half  reversal//
    traversing_the_file(fd, writefile_fd, fileSize, end+1, bufferSize, 1, buffer, total_bytes_written);                                                                                                                                                       
    } 

    close(fd);
    close(writefile_fd);
    
}

