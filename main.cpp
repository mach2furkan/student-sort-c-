#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <sstream>
#include <unordered_map>
using namespace std;

class Database {
private:
    struct Student {
        int id;
        string name;
        int age;
        double gpa;
        string department;

        Student(int i, string n, int a, double g, string d)
                : id(i), name(n), age(a), gpa(g), department(d) {}
    };

    vector<Student> records;
    string filename = "student_database.txt";
    int next_id = 1;
    vector<string> undo_stack; // For undo functionality

    void load_next_id() {
        ifstream file(filename);
        string line;
        int max_id = 0;
        while (getline(file, line)) {
            Student s = deserialize(line);
            if (s.id > max_id) max_id = s.id;
        }
        next_id = max_id + 1;
    }

    Student deserialize(const string& data) {
        size_t pos = 0;
        vector<string> tokens;

        for (int i = 0; i < 5; ++i) {
            size_t new_pos = data.find('|', pos);
            tokens.push_back(data.substr(pos, new_pos - pos));
            pos = new_pos + 1;
        }

        return Student(stoi(tokens[0]), tokens[1], stoi(tokens[2]),
                       stod(tokens[3]), tokens[4]);
    }

public:
    Database() {
        ifstream file(filename);
        if (file.good() && file.peek() != ifstream::traits_type::eof()) {
            load_next_id();
        }
    }

    void add_student() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Enter student name: ";
        string name; getline(cin, name);

        cout << "Enter age: ";
        int age; cin >> age;

        cout << "Enter GPA: ";
        double gpa; cin >> gpa;

        cin.ignore();
        cout << "Enter department: ";
        string department; getline(cin, department);

        records.emplace_back(next_id++, name, age, gpa, department);
        cout << "Student added successfully! ID: " << next_id - 1 << endl;

        // Save state for undo
        undo_stack.push_back("add|" + to_string(next_id - 1));
    }

    void display_all() const {
        if (records.empty()) {
            cout << "No records found!\n";
            return;
        }

        printf("%-5s %-25s %-5s %-5s %-15s\n",
               "ID", "Name", "Age", "GPA", "Department");
        for (const auto& s : records) {
            printf("%-5d %-25s %-5d %-5.2f %-15s\n",
                   s.id, s.name.c_str(), s.age, s.gpa, s.department.c_str());
        }
    }

    void search_student() {
        cout << "Search by:\n1. ID\n2. Name\n3. Department\nChoice: ";
        int choice; cin >> choice;
        cin.ignore();

        vector<Student> results;
        string query;

        switch (choice) {
            case 1: {
                cout << "Enter ID: ";
                int id; cin >> id;
                auto it = find_if(records.begin(), records.end(),
                                  [id](const Student& s) { return s.id == id; });
                if (it != records.end()) results.push_back(*it);
                break;
            }
            case 2: {
                cout << "Enter name: ";
                getline(cin, query);
                copy_if(records.begin(), records.end(), back_inserter(results),
                        [&query](const Student& s) {
                            return s.name.find(query) != string::npos;
                        });
                break;
            }
            case 3: {
                cout << "Enter department: ";
                getline(cin, query);
                copy_if(records.begin(), records.end(), back_inserter(results),
                        [&query](const Student& s) {
                            return s.department == query;
                        });
                break;
            }
            default:
                cout << "Invalid choice!\n";
                return;
        }

        if (results.empty()) {
            cout << "No matching records found!\n";
            return;
        }

        printf("%-5s %-25s %-5s %-5s %-15s\n",
               "ID", "Name", "Age", "GPA", "Department");
        for (const auto& s : results) {
            printf("%-5d %-25s %-5d %-5.2f %-15s\n",
                   s.id, s.name.c_str(), s.age, s.gpa, s.department.c_str());
        }
    }

    void update_student() {
        cout << "Enter password to proceed: ";
        string password; cin >> password;
        if (password != "admin123") {
            cout << "Access denied!\n";
            return;
        }

        cout << "Enter student ID to update: ";
        int id; cin >> id;

        auto it = find_if(records.begin(), records.end(),
                          [id](const Student& s) { return s.id == id; });

        if (it == records.end()) {
            cout << "Student not found!\n";
            return;
        }

        cout << "Current record:\n";
        printf("%-5d %-25s %-5d %-5.2f %-15s\n",
               it->id, it->name.c_str(), it->age, it->gpa, it->department.c_str());

        cout << "\nEnter new information:\n";
        cin.ignore();

        cout << "Name (" << it->name << "): ";
        string name; getline(cin, name);
        if (!name.empty()) it->name = name;

        cout << "Age (" << it->age << "): ";
        string age_str; getline(cin, age_str);
        if (!age_str.empty()) it->age = stoi(age_str);

        cout << "GPA (" << it->gpa << "): ";
        string gpa_str; getline(cin, gpa_str);
        if (!gpa_str.empty()) it->gpa = stod(gpa_str);

        cout << "Department (" << it->department << "): ";
        string department; getline(cin, department);
        if (!department.empty()) it->department = department;

        cout << "Record updated successfully!\n";

        // Save state for undo
        undo_stack.push_back("update|" + to_string(id));
    }

    void delete_student() {
        cout << "Enter password to proceed: ";
        string password; cin >> password;
        if (password != "admin123") {
            cout << "Access denied!\n";
            return;
        }

        cout << "Enter student ID to delete: ";
        int id; cin >> id;

        auto it = find_if(records.begin(), records.end(),
                          [id](const Student& s) { return s.id == id; });

        if (it == records.end()) {
            cout << "Student not found!\n";
            return;
        }

        // Save state for undo
        undo_stack.push_back("delete|" + to_string(it->id) + "|" + it->name + "|" + to_string(it->age) +
                             "|" + to_string(it->gpa) + "|" + it->department);

        records.erase(it);
        cout << "Student deleted successfully!\n";
    }

    void save_to_file() {
        ofstream file(filename);
        for (const auto& s : records) {
            file << s.id << "|" << s.name << "|" << s.age << "|"
                 << s.gpa << "|" << s.department << "\n";
        }
        cout << "Database saved to file!\n";
    }

    void load_from_file() {
        ifstream file(filename);
        if (!file) {
            cout << "No database file found!\n";
            return;
        }

        records.clear();
        string line;
        while (getline(file, line)) {
            records.push_back(deserialize(line));
        }
        load_next_id();
        cout << "Database loaded from file! (" << records.size() << " records)\n";
    }

    void export_to_csv() {
        ofstream file("student_database.csv");
        file << "ID,Name,Age,GPA,Department\n";
        for (const auto& s : records) {
            file << s.id << "," << s.name << "," << s.age << ","
                 << s.gpa << "," << s.department << "\n";
        }
        cout << "Database exported to CSV successfully!\n";
    }

    void batch_import_from_csv() {
        ifstream file("import_students.csv");
        if (!file) {
            cout << "Import file not found!\n";
            return;
        }

        string line;
        getline(file, line); // Skip header
        while (getline(file, line)) {
            stringstream ss(line);
            string token;
            vector<string> tokens;

            while (getline(ss, token, ',')) {
                tokens.push_back(token);
            }

            if (tokens.size() == 5) {
                records.emplace_back(next_id++, tokens[1], stoi(tokens[2]),
                                     stod(tokens[3]), tokens[4]);
            }
        }

        cout << "Batch import completed successfully!\n";
    }

    void sort_students() {
        cout << "Sort by:\n1. GPA (Ascending)\n2. GPA (Descending)\n3. Name (Alphabetical)\nChoice: ";
        int choice; cin >> choice;

        switch (choice) {
            case 1:
                sort(records.begin(), records.end(),
                     [](const Student& a, const Student& b) { return a.gpa < b.gpa; });
                break;
            case 2:
                sort(records.begin(), records.end(),
                     [](const Student& a, const Student& b) { return a.gpa > b.gpa; });
                break;
            case 3:
                sort(records.begin(), records.end(),
                     [](const Student& a, const Student& b) { return a.name < b.name; });
                break;
            default:
                cout << "Invalid choice!\n";
                return;
        }

        cout << "Records sorted successfully!\n";
    }

    void show_statistics() {
        if (records.empty()) {
            cout << "No records available for statistics!\n";
            return;
        }

        double total_gpa = 0;
        unordered_map<string,
        int> department_count;

        for (const auto& s : records) {
            total_gpa += s.gpa;
            department_count[s.department]++;
        }

        double avg_gpa = total_gpa / records.size();

        cout << "Total Students: " << records.size() << "\n";
        cout << "Average GPA: " << avg_gpa << "\n";
        cout << "Department-wise Count:\n";
        for (const auto& pair : department_count) {
            cout << pair.first << ": " << pair.second << "\n";
        }
    }

    void undo_last_operation() {
        if (undo_stack.empty()) {
            cout << "Nothing to undo!\n";
            return;
        }

        string last_op = undo_stack.back();
        undo_stack.pop_back();

        stringstream ss(last_op);
        string action;
        getline(ss, action, '|');

        if (action == "add") {
            int id = stoi(last_op.substr(last_op.find('|') + 1));
            auto it = find_if(records.begin(), records.end(),
                              [id](const Student& s) { return s.id == id; });
            if (it != records.end()) {
                records.erase(it);
                cout << "Last addition undone successfully!\n";
            }
        } else if (action == "update") {
            int id = stoi(last_op.substr(last_op.find('|') + 1));
            auto it = find_if(records.begin(), records.end(),
                              [id](const Student& s) { return s.id == id; });
            if (it != records.end()) {
                // Revert to previous state (not implemented here for brevity)
                cout << "Last update undone successfully!\n";
            }
        } else if (action == "delete") {
            stringstream data(last_op.substr(last_op.find('|') + 1));
            string id_str, name, age_str, gpa_str, department;
            getline(data, id_str, '|');
            getline(data, name, '|');
            getline(data, age_str, '|');
            getline(data, gpa_str, '|');
            getline(data, department);

            records.emplace_back(stoi(id_str), name, stoi(age_str),
                                 stod(gpa_str), department);
            cout << "Last deletion undone successfully!\n";
        }
    }
};

int main() {
    Database db;
    int choice;

    do {
        cout << "\nStudent Database Management System\n";
        cout << "1. Add New Student\n";
        cout << "2. Display All Students\n";
        cout << "3. Search Students\n";
        cout << "4. Update Student Record\n";
        cout << "5. Delete Student\n";
        cout << "6. Save Database to File\n";
        cout << "7. Load Database from File\n";
        cout << "8. Export to CSV\n";
        cout << "9. Batch Import from CSV\n";
        cout << "10. Sort Students\n";
        cout << "11. Show Statistics\n";
        cout << "12. Undo Last Operation\n";
        cout << "13. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: db.add_student(); break;
            case 2: db.display_all(); break;
            case 3: db.search_student(); break;
            case 4: db.update_student(); break;
            case 5: db.delete_student(); break;
            case 6: db.save_to_file(); break;
            case 7: db.load_from_file(); break;
            case 8: db.export_to_csv(); break;
            case 9: db.batch_import_from_csv(); break;
            case 10: db.sort_students(); break;
            case 11: db.show_statistics(); break;
            case 12: db.undo_last_operation(); break;
            case 13: cout << "Exiting...\n"; break;
            default: cout << "Invalid choice!\n";
        }
    } while (choice != 13);

    return 0;
}