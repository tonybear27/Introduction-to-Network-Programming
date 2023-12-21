#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <curl/curl.h>
#include <fstream>

using namespace std;

/* Header */ 
struct binflag_header_t {
    uint64_t magic;     // 'BINFLAG\x00' (in big-endian)
    uint32_t datasize;  // in big-endian
    uint16_t n_blocks;  // in big-endian
    uint16_t zeros;
};

/* Blocks */
typedef struct {
    uint32_t offset;        /* in big-endian */
    uint16_t cksum;         /* XOR'ed results of each 2-byte unit in payload */
    uint16_t length;        /* ranges from 1KB - 3KB, in big-endian */
    uint8_t  payload[0];
} __attribute((packed)) block_t;

/* Flag Information */
typedef struct {
   uint16_t length;        /* length of the offset array, in big-endian */
   uint32_t offset[0];     /* offset of the flags, in big-endian */
} __attribute((packed)) flag_t;


/* Download Challange.bin */
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ostream* file = static_cast<ostream*>(userp);
    file->write(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}
void downloadFile() {
    CURL* curl;
    const char* url = "https://inp.zoolab.org/binflag/challenge?id=110700045";

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Open a local file to save the downloaded content
        ofstream outputFile("challange.bin", ios::binary);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outputFile);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            cerr << "Failed to download the file: " << curl_easy_strerror(res) << endl;
        else 
            cout << "File downloaded successfully." << endl;

        curl_easy_cleanup(curl);
    } else
        cerr << "Failed to initialize libcurl." << endl;
}

/* From little-endian to big-endian */
template <typename T>
T swapEndianness(T value) {
    int numBytes = sizeof(T);
    T result = 0;

    for (int i = 0; i < numBytes; i++)
        result = (result << 8) | ((value >> (i * 8)) & 0xFF);

    return result;
}

/* From decimal to hexdecimal */
string dec2hex(int n, bool flag) {
    stringstream ss;
    ss<<hex<<static_cast<int>(n);
    string hexString = ss.str();
    if (hexString.length()==1 && flag)
        hexString="0"+hexString;
    return hexString;
}

/* Checksum verification */
bool verify(uint16_t cksum, vector<string> s) {
    int result = stoi(s[0], nullptr, 16);

    for (size_t i = 1; i < s.size(); ++i) {
        int num = std::stoi(s[i], nullptr, 16);
        result ^= num;
    }

    if (dec2hex(cksum, 1)==dec2hex(result, 1))
        return 1;
    return 0;
}

/* Concate two hexdecimal together , ex: AA BB => AABB */
void concate(vector<uint8_t> bytes, vector<string> &payload) {
    for (uint16_t i = 0; i < bytes.size(); ++i) {
        string hexString=dec2hex(bytes[i], 1);
        if (!(i%2))
            payload.push_back(hexString);
        else {
            string temp=payload.back();
            temp+=hexString;
            payload.pop_back();
            payload.push_back(temp);
        }
    }
}

void sentFlag(const char* s)
{
    CURL *curl;
    CURLcode res;

    for (int i=0; i<strlen(s); i++)
        cout<<s[i];
    puts("");

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, s);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}


int main() {

    downloadFile();

    /* Read bin file */
    ifstream input("challange.bin", ios::binary);
    if (!input) {
        cerr << "Error opening the file." << endl;
        return 1;
    }

    /* Read header information */
    binflag_header_t header;
    input.read(reinterpret_cast<char*>(&header), sizeof(header));
    header.magic = swapEndianness<uint64_t>(header.magic);
    header.datasize = swapEndianness<uint32_t>(header.datasize);
    header.n_blocks = swapEndianness<uint16_t>(header.n_blocks);
    header.zeros=swapEndianness<uint16_t>(header.zeros);

    block_t *blocks[header.n_blocks];
    vector<bool> valid(header.n_blocks);
    vector<uint8_t> data(header.datasize);

    for (uint16_t k=0; k<header.n_blocks; k++) {

        /* Read the block information and then transform to big-endian */
        block_t block;
        input.read(reinterpret_cast<char*>(&block), sizeof(block));
        block.offset=swapEndianness<uint32_t>(block.offset);
        block.cksum=swapEndianness<uint16_t>(block.cksum);
        block.length=swapEndianness<uint16_t>(block.length);
        uint16_t len=block.length;
        
        /* Store the read payload to block->payload */
        vector<uint8_t> bytes(len);
        vector<string> Payload;
        blocks[k] = static_cast<block_t*>(::operator new(sizeof(block_t) + block.length));
        blocks[k]->offset=block.offset;
        blocks[k]->cksum=block.cksum;
        blocks[k]->length=block.length;
        for (uint16_t i = 0; i < len; i++) {
            uint8_t byte;
            input.read(reinterpret_cast<char*>(&byte), sizeof(byte));
            bytes[i] = byte;
        }

        concate(bytes, Payload);
        
        copy(bytes.begin(), bytes.end(), blocks[k]->payload);

        /* Verify the block */
        if(verify(block.cksum, Payload))
            valid[k]=1;
        else
            valid[k]=0;
    }

    /* Read flag information */
    flag_t flag;
    input.read(reinterpret_cast<char*>(&flag), sizeof(flag));
    flag.length=swapEndianness<uint16_t>(flag.length);
    vector<uint32_t> seq(flag.length);
    for (uint16_t i = 0; i < flag.length; ++i) {
            uint32_t byte;
            input.read(reinterpret_cast<char*>(&byte), sizeof(byte));
            byte=swapEndianness<uint32_t>(byte);
            seq[i] = byte;
        }

    /* Put the block information to dictionary file D */
    for (int i=0; i<header.n_blocks; i++) {
        if (valid[i]) {
            for (int k=0; k<blocks[i]->length; k++) {
                data[blocks[i]->offset+k]=blocks[i]->payload[k];
            }
        }
    }

    /* Decode the flag */
    string ans="";
    for(int i=0; i<seq.size(); i++) {
        string s1=dec2hex(int(data[seq[i]]), 0);
        string s2=dec2hex(int(data[seq[i]+1]), 0);
        ans+=s1+s2;
    }
    /* Sent the decoded flag to the corresponding URL */
    string URL="https://inp.zoolab.org/binflag/verify?v=";
    cout<<ans<<endl;
    string message=URL+ans;
    sentFlag(message.c_str());
    input.close();

    return 0;
}