
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <limits>
using namespace std;
 
class Student {
protected:
    int rollNo;
    string name;
public:
    Student(int r = 0, const string& n = "") : rollNo(r), name(n) {}
    virtual void display() const {
        cout << "Roll Number: " << rollNo << "\nName: " << name << '\n';
    }
    virtual ~Student() = default;
};
 
class Result : public Student {
private:
    vector<double> marks;
public:
    Result(int r, const string& n, const vector<double>& m) : Student(r, n), marks(m) {
        if (marks.empty()) throw invalid_argument("At least one subject mark is required.");
        for (double mark : marks) {
            if (mark < 0 || mark > 100)
                throw invalid_argument("Marks must be between 0 and 100.");
        }
    }
    double total() const {
        double sum = 0;
        for (double mark : marks) sum += mark;
        return sum;
    }
    double percentage() const { return total() / marks.size(); }
    string resultStatus() const { return percentage() >= 40 ? "PASS" : "FAIL"; }
    void display() const override {
        Student::display();
        cout << "Subject Marks: ";
        for (double mark : marks) cout << mark << "  ";
        cout << "\nTotal Marks: " << total()
             << "\nPercentage: " << fixed << setprecision(2) << percentage() << "%"
             << "\nResult: " << resultStatus() << "\n";
    }
    string toFileRecord() const {
        ostringstream out;
        out << rollNo << ',' << name << ',';
        for (size_t i = 0; i < marks.size(); ++i) {
            if (i) out << ';';
            out << marks[i];
        }
        return out.str();
    }
    int getRollNo() const { return rollNo; }
    const string& getName() const { return name; }
};
 
void saveRecords(const vector<Result>& students, const string& filename) {
    ofstream out(filename);
    if (!out) throw runtime_error("Unable to open file for writing.");
    for (const Result& student : students) out << student.toFileRecord() << '\n';
}
 
vector<Result> loadRecords(const string& filename) {
    ifstream in(filename);
    if (!in) throw runtime_error("Unable to open file for reading.");
    vector<Result> loaded;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string rollText, name, marksText;
        if (!getline(ss, rollText, ',') || !getline(ss, name, ',') || !getline(ss, marksText))
            throw runtime_error("Invalid record format in file.");
        vector<double> marks;
        stringstream ms(marksText);
        string value;
        while (getline(ms, value, ';')) marks.push_back(stod(value));
        loaded.emplace_back(stoi(rollText), name, marks);
    }
    return loaded;
}
 
int main() {
    vector<Result> students;
    int count;
    cout << "STUDENT RESULT MANAGEMENT SYSTEM\n";
    cout << "Enter number of students: ";
    if (!(cin >> count) || count <= 0) {
        cerr << "Error: Enter a positive number of students.\n";
        return 1;
    }
 
    const int SUBJECTS = 3;
    for (int i = 0; i < count; ++i) {
        int roll;
        string name;
        vector<double> marks(SUBJECTS);
        cout << "\nStudent " << i + 1 << "\nRoll number: ";
        cin >> roll;
        cout << "Name (one word): ";
        cin >> name;
        try {
            for (int j = 0; j < SUBJECTS; ++j) {
                cout << "Marks in subject " << j + 1 << " (0-100): ";
                if (!(cin >> marks[j])) throw invalid_argument("Marks must be numeric.");
                if (marks[j] < 0 || marks[j] > 100)
                    throw out_of_range("Invalid marks: enter a value from 0 to 100.");
            }
            students.emplace_back(roll, name, marks);
        } catch (const exception& e) {
            cerr << "Input error: " << e.what() << "\nPlease restart and enter valid data.\n";
            return 1;
        }
    }
 
    cout << "\n===== STUDENT RESULTS =====\n";
    for (const Result& student : students) {
        const Student& baseRef = student; // Base-class reference demonstrates runtime polymorphism.
        baseRef.display();
        cout << "--------------------------\n";
    }
 
    try {
        saveRecords(students, "student_records.txt");
        cout << "Records saved to student_records.txt\n";
        vector<Result> storedStudents = loadRecords("student_records.txt");
        cout << "\n===== RECORDS RETRIEVED FROM FILE =====\n";
        for (const Result& student : storedStudents) student.display();
 
        double cutoff;
        cout << "\nEnter percentage threshold: ";
        if (!(cin >> cutoff) || cutoff < 0 || cutoff > 100)
            throw invalid_argument("Threshold must be between 0 and 100.");
        cout << "\nStudents scoring above " << cutoff << "%:\n";
        int found = 0;
        for_each(storedStudents.begin(), storedStudents.end(),
                 [cutoff, &found](const Result& student) {
                     if (student.percentage() > cutoff) {
                         cout << student.getRollNo() << " - " << student.getName()
                              << " (" << fixed << setprecision(2)
                              << student.percentage() << "%)\n";
                         ++found;
                     }
                 });
        if (found == 0) cout << "No students scored above the specified threshold.\n";
    } catch (const exception& e) {
        cerr << "File/processing error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}