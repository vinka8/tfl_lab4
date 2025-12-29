#include <bits/stdc++.h>
using namespace std;

mt19937 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());

int randint(int l, int r) {
    uniform_int_distribution<int> dist(l, r);
    return dist(rng);
}

//случайные слова, принадлежащие языку
string generate_special_word(int iter, int block, int num_b) {
    int iterations = randint(0, iter);
    string word;

    int n = randint(1, num_b);
    word += string(n, 'b');
    int m = 0;

    for (int it = 0; it < iterations; it++) {
        m = randint(1, n);
        int t = randint(0, block);
        for (int i = 1; i <= t; i++) {
            word += string(i, 'a');
            word += string(m, 'b');
            word += string(i, 'a');
        }
        n = randint(m, num_b);
        word += string(n, 'b');
    }
    return word;
}

//случайные слова, не принадлежащие языку
pair<vector<string>, vector<string>> generate_words(int max_length, int count, int iter, int block, int num_b) {
    vector<string> random_words, special_words;

    for (int i = 0; i < count; i++) {
        int len = randint(max_length / 2, max_length);
        string w;
        for (int j = 0; j < len; j++) {
            w += (randint(0, 1) ? 'a' : 'b');
        }
        random_words.push_back(w);
        special_words.push_back(generate_special_word(iter, block, num_b));
    }
    return {random_words, special_words};
}

//рекурсивный наивный парсер
bool parse_language(const string& s) {
    int n = s.size();

    function<bool(int, unordered_map<int,string>)> parse = [&](int pos, unordered_map<int,string> groups) -> bool {
        if (pos == n) return true;
        auto local = groups;
        if (local.count(1) && s[pos] == 'a') {
            int pos1 = pos;
            int length = local[1].size();

            bool all_same = true;
            for (char c : local[1])
                if (c != local[1][0]) all_same = false;

            if (all_same) {
                int p = pos + 1;
                while (p < n && s[p] == 'b') p++;
                if (p >= n || (p - pos - 1) > length ||
                    (p - pos - 1) < 1 || s[p] != 'a')
                    pos1 = -1;
                else {
                    local[1] = string(p - pos - 1, 'b');
                    local[2] = local[1];
                }
            }
            else if (pos + length + 1 >= n ||
                     s.substr(pos + 1, length) != local[1] ||
                     string() + s[pos] + s[pos + length + 1] != "aa")
                pos1 = -1;

            if (pos1 != -1) {
                length = local[1].size();
                pos1 += length + 2;

                if (local.count(2)) {
                    int len2 = local[2].size();
                    bool ok = false;
                    for (int k = 0; pos1 + k + len2 <= n; k++) {
                        if (s.substr(pos1 + k, len2) == local[2] &&
                            s.substr(pos1, k) == string(k, 'a')) {
                            ok = true;
                            break;
                        }
                    }
                    if (!ok) pos1 = -1;
                } else pos1 = -1;

                if (pos1 != -1 && pos1 < n)
                    local[1] = s.substr(pos, pos1 - pos);
                else pos1 = -1;
            }

            if (pos1 != -1)
                return parse(pos1, local);
        }

        else if (pos < n && s[pos] == 'b') {
            int p = pos, nb = 0;
            while (p < n && s[p] == 'b') {
                p++; nb++;
            }
            local[1] = string(nb, 'b');
            local[2] = string(nb, 'b');
            return parse(p, local);
        }

        return false;
    };

    return parse(0, {});
}

// Оптимизированный парсер со сложностью O(n)
bool optimized_parse_language(const string& s) {
    int n = s.size();
    
    if (n == 0) return true;

    if (s.front() != 'b' || s.back() != 'b') return false;

    int pos = 0, m_b = 0, n_a = 0;
    while (pos < n && s[pos] == 'b') {
        m_b++; 
        pos++;
    }

    while (pos < n) {
        if (s[pos] == 'b') {
            int k = 0;
            while (pos < n && s[pos] == 'b') {
                k++; 
                pos++;
            }
            if (n_a && k < m_b) {
                return false;
            }
            n_a = 0; 
            m_b = k;
            continue;
        }

        pos++;
        for (int i = 0; i < n_a; i++) {
            if (pos >= n || s[pos++] != 'a') {
                return false;
            }
        }

        int k = 0;
        while (pos < n && s[pos] == 'b') {
            k++; 
            pos++;
        }
        if (k == 0 || k > m_b || (k != m_b && n_a)) {
            return false;
        }

        if (!n_a && m_b != k) {
            m_b = k;
        }

        for (int i = 0; i < n_a; i++){
            if (pos >= n || s[pos++] != 'a') {
                return false;
            }
        }

        if (pos >= n || s[pos++] != 'a') {
            return false;
        }
        n_a++;
    }
    return true;
}

void plot() {
    const int count = 500;
    const int block_size = 50; 
    int iter = 200;
    int block = 100;
    int num_b = 200;
    int random_length = 200000;
    auto [random_words, special_words] = generate_words(random_length, count, iter, block, num_b);

    cout << "type,total_length,recursive_time,optimized_time\n";
    auto test_block = [&](const vector<string>& words, const string& type) {
        for (int start = 0; start < count/2; start += block_size) {
            int end = min(start + block_size, count/2);
            size_t total_length = 0;
            for (int i = start; i < end; i++)
                total_length += words[i].size();

            auto t1 = chrono::high_resolution_clock::now();
            for (int i = start; i < end; i++)
                parse_language(words[i]);
            auto t2 = chrono::high_resolution_clock::now();

            for (int i = start; i < end; i++)
                optimized_parse_language(words[i]);
            auto t3 = chrono::high_resolution_clock::now();

            double trec = chrono::duration<double>(t2 - t1).count();
            double topt = chrono::duration<double>(t3 - t2).count();
            total_length =  total_length / block_size;

            cout << type << "," << total_length << "," << trec << "," << topt << "\n";
        }
    };

    test_block(special_words, "in_language");
    test_block(random_words, "not_in_language");
}


int main() {
    plot();
    auto [random_words, special_words] = generate_words(300, 200000, 15, 10, 20);

    for (auto& w : random_words) {
        bool a = optimized_parse_language(w);
        bool b = parse_language(w);
        if (a != b) {
            cout << a << "  " << b << "  " << w << endl;
            return 0;
        }
    }

    auto t1 = chrono::high_resolution_clock::now();
    for (auto& s : random_words) {
        parse_language(s);
    }
    auto t2 = chrono::high_resolution_clock::now();
    cout << chrono::duration<double>(t2 - t1).count() << endl;
    t1 = chrono::high_resolution_clock::now();
    for (auto& s : random_words) {
        optimized_parse_language(s);
    }
    t2 = chrono::high_resolution_clock::now();
    cout << chrono::duration<double>(t2 - t1).count() << endl;

    for (auto& w : special_words) {
        bool a = optimized_parse_language(w);
        bool b = parse_language(w);
        if (a != b || a != true)  {
            cout << a << "  " << b << "  " << w << endl;
            return 0;
        }
    }

    t1 = chrono::high_resolution_clock::now();
    for (auto& s : special_words) {
        parse_language(s);
    }
    t2 = chrono::high_resolution_clock::now();
    cout << chrono::duration<double>(t2 - t1).count() << endl;
    t1 = chrono::high_resolution_clock::now();
    for (auto& s : special_words) {
        optimized_parse_language(s);
    }
    t2 = chrono::high_resolution_clock::now();
    cout << chrono::duration<double>(t2 - t1).count() << endl;

    cout << "Парсеры работают эквивалентно" << endl;
}
