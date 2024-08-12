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

void  create_directory() {
    mkdir("Assignment1", 0755);
}

int create_file(const char* path) {
    return open(path,  O_RDWR | O_CREAT | O_TRUNC, 0666);
}

int open_file(const char* path) {
    return open(path, O_RDONLY);
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


string setPrecision(string progress) {
    string precise;
    int cnt = 0;
    bool flag = false;
    for (auto it : progress) {
        if(it == '.') {
            cnt++;
            flag = !flag;
        }
        if (cnt == 3 && flag) break;

        if (!flag) precise.push_back(it);
    }
    return precise;
}

void displayProgress(ssize_t data_written, ssize_t fileSize, string& percentage ) {
   double  progress = (static_cast<double>(data_written)) / fileSize * 100;
    percentage =  setPrecision(to_string(progress)) + "%";
}

ssize_t write_to_file_with_given_size(int fd, int writefile_fd ,ssize_t offSet, char* buffer, ssize_t bytes_to_read, int rev) {
        lseek(fd, offSet, SEEK_SET);
         ssize_t curr_read = read(fd, buffer, bytes_to_read);
         if (rev) reverse_chunks(buffer, curr_read);
         return write(writefile_fd, buffer, curr_read);

}

int main( int argc, char *argv[]) {
    char* input_file;
    ssize_t curr_chunk;
    ssize_t start = 0;
    ssize_t end = 0;


    //bytes//
    ssize_t bufferSize = 1024 * 1024;
    char buffer[bufferSize];
    off_t fileSize;

    if (argv[2] == "1" && argc < 5) {
        perror("provide the start and end indices");
    } else if (argc < 3) {
        perror("Flag 0 : provide input in the following format ./aout <input file> <flag>");   
    }
    input_file = argv[1];

    int flag = convert_to_integer(argv[2]);
    if (flag && argc == 5) {
        start = convert_to_integer(argv[3]);
        end = convert_to_integer(argv[4]);
    }


    //input file//
    int fd = open_file(input_file);
    fileSize = getfilesize(fd);

    //file dir//
    create_directory();

    //output_file//
    int writefile_fd = create_file("Assignment1/output.txt");
    size_t total = 0;

    if (!flag) {
         //traversing file and revrsing the chunks//
        ssize_t offSet = fileSize;
        ssize_t bytes_to_read = bufferSize;
        string percentage;
        ssize_t total_bytes_written = 0;

        
        int rem_chunk = fileSize % bufferSize;
        for (ssize_t offSet = fileSize-bufferSize; offSet>=0; offSet-=bytes_to_read) {
            ssize_t data_written = write_to_file_with_given_size(fd, writefile_fd, offSet, buffer, bytes_to_read, 1);
                total_bytes_written += data_written;
                displayProgress(total_bytes_written, fileSize, percentage);
                write(1, "\r\x1b[2K", 5);
                write(1, percentage.c_str(), percentage.size());
        }

        // if rem_chunk exists read the last chunk and write the file//
        if (rem_chunk) {
            ssize_t data_written = write_to_file_with_given_size(fd, writefile_fd, 0, buffer, rem_chunk, 1);
            total_bytes_written += data_written;
            displayProgress(total_bytes_written, fileSize, percentage);
            write(1, "\r\x1b[2K", 5);
            write(1, percentage.c_str(), percentage.size());

        }
    } else {
        //left half reversal//
        ssize_t offSet;
        ssize_t bytes_to_read = bufferSize;
        string percentage;
        ssize_t  total_bytes_written = 0;
        int rem_chunk = start % bufferSize;
        for (offSet = start-bufferSize; offSet>=0; offSet-=bytes_to_read) {
            ssize_t data_written = write_to_file_with_given_size(fd, writefile_fd, offSet, buffer, bytes_to_read, 1);
                total_bytes_written += data_written;
                displayProgress(total_bytes_written, fileSize, percentage);
                write(1, "\r\x1b[2K", 5);
                write(1, percentage.c_str(), percentage.size());
        }

        // if rem_chunk exists read the last chunk and write the file//
        if (rem_chunk) {
            ssize_t data_written = write_to_file_with_given_size(fd, writefile_fd, 0, buffer, rem_chunk, 1);
            total_bytes_written += data_written;
            displayProgress(total_bytes_written, fileSize, percentage);
            write(1, "\r\x1b[2K", 5);
            write(1, percentage.c_str(), percentage.size());

        }

//--------------------------------------------------------------------------------------------------------------------------------//
        //middle chunk reversal//
            ssize_t midChunk = abs(end - start)+1;
            rem_chunk = midChunk % bufferSize;
            bytes_to_read = bufferSize;                                                                                                                                                                      
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

// //===============================================================================================================================//

        //right half  reversal//
        ssize_t rightChunk = (fileSize - end - 1);
        rem_chunk = rightChunk % bufferSize;
        bytes_to_read = bufferSize;     

        for (offSet = fileSize-bufferSize; offSet>end; offSet-=bytes_to_read) {
                
                ssize_t data_written = write_to_file_with_given_size(fd, writefile_fd, offSet, buffer, bytes_to_read, 1);
                total_bytes_written += data_written;
                displayProgress(total_bytes_written, fileSize, percentage);
                write(1, "\r\x1b[2K", 5);
                write(1, percentage.c_str(), percentage.size());
        }
        // if rem_chunk exists read the last chunk and write the file//
        if (rem_chunk) {
            ssize_t data_written = write_to_file_with_given_size(fd, writefile_fd, end+1, buffer, rem_chunk, 1);
            total_bytes_written += data_written;
            displayProgress(total_bytes_written, fileSize, percentage);
            write(1, "\r\x1b[2K", 5);
            write(1, percentage.c_str(), percentage.size());

        }                                                                                                                                                                

      } 
   
    close(fd);
    close(writefile_fd);
}

