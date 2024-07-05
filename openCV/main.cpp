#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <vector>
#include <unordered_set>
#include <iomanip>

using namespace cv;
using namespace std;

const string databasePath = "H:/Development/SATS/BlockBallot/Database/";
const string samplePath = "H:/Development/SATS/BlockBallot/ImgSample/";
const string decryptedPath = "H:/Development/SATS/BlockBallot/BlockTum/";

class SimpleEncryption
{
private:
    char key; 

public:
    SimpleEncryption(char key) : key(key) {}

    string encrypt(const string& plaintext)
    {
        string ciphertext = plaintext;
        for (size_t i = 0; i < ciphertext.size(); ++i)
        {
            ciphertext[i] = ciphertext[i] ^ key;
        }
        return ciphertext;
    }

    string decrypt(const string& ciphertext)
    {
        return encrypt(ciphertext); 
    }
};

class UserData
{
private:
    string name;
    int age;
    string cnic;
    string politicalParty;
    string imagePath;

public:
    UserData(const string& name, int age, const string& cnic, const string& politicalParty, const string& imagePath)
        : name(name), age(age), cnic(cnic), politicalParty(politicalParty), imagePath(imagePath) {}

    string getName() const { return name; }
    string getCNIC() const { return cnic; }
    string getPoliticalParty() const { return politicalParty; }
    string getImagePath() const { return imagePath; }

    string encryptData(SimpleEncryption& encryption) const
    {
        stringstream ss;
        ss << "Name: " << name << endl;
        ss << "Age: " << age << endl;
        ss << "CNIC: " << cnic << endl;
        ss << "Political Party: " << politicalParty << endl;
        ss << "Image Path: " << imagePath << endl;
        string plaintext = ss.str();

        return encryption.encrypt(plaintext);
    }

    void decryptData(const string& encryptedData, SimpleEncryption& encryption) const
    {
        string decryptedText = encryption.decrypt(encryptedData);
        cout << "Decrypted Data:\n"
            << decryptedText << endl;

        string decryptedFilePath = decryptedPath + "decrypted_user_" + name + ".txt";
        ofstream file(decryptedFilePath);
        if (file.is_open())
        {
            file << decryptedText;
            file.close();
            cout << "Decrypted data saved successfully at: " << decryptedFilePath << endl;
        }
        else
        {
            cout << "Unable to open file to save decrypted data!" << endl;
        }
    }
};

class BlockData
{
private:
    int blockNumber;

public:
    BlockData(int blockNumber) : blockNumber(blockNumber) {}

    void displayBlockDetails(const string& blockHash, const string& timestamp) const
    {
        cout << "Block Details for Block " << blockNumber << ":" << endl;
        cout << "--------------------------" << endl;
        cout << "Block Hash: " << blockHash << endl;
        cout << "Timestamp: " << timestamp << endl;
        cout << "--------------------------" << endl;
    }
};

class VotingSystem
{
private:
    CascadeClassifier faceCascade;
    vector<Mat> storedFaces;
    vector<string> storedNames;
    unordered_set<size_t> processedFaces;
    int blockNumber;

public:
    VotingSystem(const string& faceCascadePath) : faceCascade(faceCascadePath), blockNumber(0) {}

    void start()
    {
        VideoCapture cap(0);
        if (!cap.isOpened())
        {
            cout << "Error opening video capture!" << endl;
            exit(1);
        }

        namedWindow("Face Recognition", WINDOW_NORMAL);
        resizeWindow("Face Recognition", 800, 600);

        bool continueRecognition = true;

        while (continueRecognition)
        {
            Mat frame;
            cap >> frame;

            resize(frame, frame, Size(frame.cols / 2, frame.rows / 2));

            Mat grayFrame;
            cvtColor(frame, grayFrame, COLOR_BGR2GRAY);

            vector<Rect> faces;
            faceCascade.detectMultiScale(grayFrame, faces, 1.1, 3, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

            for (size_t i = 0; i < faces.size(); ++i)
            {
                Rect face = faces[i];
                rectangle(frame, face, Scalar(0, 255, 0), 2);

                size_t faceHash = hash<string>{}(to_string(face.x) + "_" + to_string(face.y) + "_" + to_string(face.width) + "_" + to_string(face.height));
                int faceHashInt = static_cast<int>(faceHash);

                if (processedFaces.count(faceHash) > 0)
                {
                    cout << "Face already processed." << endl;
                    continue;
                }

                Mat newFace = grayFrame(face);

                string name, cnic, politicalParty;
                int age = -1;

                cout << "Press 1 to enter details for the detected face and 0 for exit: " << endl;
                char userChoice;
                cin >> userChoice;

                if (userChoice == '0')
                {
                    continueRecognition = false;
                    break;
                }
                if (userChoice == '1')
                {
                    cout << "Enter name for face " << i + 1 << ": ";
                    cin.ignore();
                    getline(cin, name);
                    cout << "Enter age for face " << i + 1 << ": ";
                    cin >> age;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Enter CNIC for face " << i + 1 << ": ";
                    getline(cin, cnic);
                    cout << "Enter political party for face " << i + 1 << ": ";
                    getline(cin, politicalParty);

                    string imagePath = samplePath + name + "_" + to_string(time(0)) + ".png";
                    imwrite(imagePath, newFace);

                    UserData user(name, age, cnic, politicalParty, imagePath);
                    SimpleEncryption encryption('K');

                    string encryptedData = user.encryptData(encryption);
                    string filePath = databasePath + "user_" + name + ".txt";

                    ofstream file(filePath);
                    if (file.is_open())
                    {
                        file << encryptedData;
                        file.close();
                        cout << "User data saved successfully." << endl;

                        cout << "Entered Details:" << endl;
                        cout << "Name: " << name << endl;
                        cout << "Age: " << age << endl;
                        cout << "CNIC: " << cnic << endl;
                        cout << "Political Party: " << politicalParty << endl;
                    }
                    else
                    {
                        cout << "Unable to open file to save user data!" << endl;
                    }

                    user.decryptData(encryptedData, encryption); // Decrypt and save decrypted data
                    BlockData blockData(++blockNumber);
                    string blockHash = user.getName() + user.getCNIC() + user.getPoliticalParty();
                    string timestamp = getCurrentTimestamp();
                    blockData.displayBlockDetails(blockHash, timestamp);

                    storedFaces.push_back(newFace.clone());
                    storedNames.push_back(name);
                    processedFaces.insert(faceHash);
                }
            }

            imshow("Face Recognition", frame);
            if (waitKey(30) == 'q')
            {
                break;
            }
        }

        cap.release();
        destroyAllWindows();
    }

private:
    string getCurrentTimestamp() const
    {
        time_t now = time(0);
        struct tm timeinfo;
        localtime_s(&timeinfo, &now);
        char buffer[26];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
        return string(buffer);
    }
};

int main()
{
    string faceCascadePath = "H:/Development/haarcascade_frontalface_default.xml";
    VotingSystem votingSystem(faceCascadePath);
    votingSystem.start();

    return 0;
}

