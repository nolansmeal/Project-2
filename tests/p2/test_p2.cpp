// tests/p2/test_p2.cpp
//
// YOUR test suite goes here. At least 12 assert-based test cases — see
// spec §5 for the required categories and the sample test for the
// expected level of rigor.
//
// This file is a stub so the project builds out of the box; replace the
// body of main() with your own tests.

#include "core/conversation.h"
#include "core/message.h"
#include "core/sentinel_scanner.h"
#include "harness/harness.h"
#include "model/replay_client.h"
#include "model/scripted_client.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

class TestInput : public InputSource {
public:
    TestInput(vector<string> lines) {
        lines_ = lines;
        index_ = 0;
    }

    string read_line() override {
        if (index_ >= lines_.size()) {
            return "";
        }

        string line = lines_[index_];
        index_++;

        return line;
    }

    bool is_eof() const override {
        return index_ >= lines_.size();
    }

private:
    vector<string> lines_;
    size_t index_;
};

class TestOutput : public OutputSink {
public:
    void write(string_view text) override {
        output_ += string(text);
    }

    string output() const {
        return output_;
    }

private:
    string output_;
};

void testEmptyConversation() {
    Conversation conv;

    assert(conv.size() == 0);
    assert(conv.begin() == conv.end());

    bool threw = false;

    try {
        conv.at(0);
    }
    catch (const out_of_range&) {
        threw = true;
    }

    assert(threw);
}

void testSystemMessageOrdering() {
    Conversation conv;

    conv.append(Message(Role::System, "System message"));
    conv.append(Message(Role::User, "Hello"));
    conv.append(Message(Role::Assistant, "Hi"));

    assert(conv.size() == 3);
    assert(conv.at(0).role() == Role::System);
    assert(conv.at(1).role() == Role::User);
    assert(conv.at(2).role() == Role::Assistant);
}

void testCopyConstructor() {
    Conversation original;

    original.append(Message(Role::User, "Hello"));
    original.append(Message(Role::Assistant, "Hi"));

    Conversation copy(original);

    assert(copy.size() == original.size());

    assert(copy.at(0).content() == "Hello");
    assert(copy.at(1).content() == "Hi");

    assert(copy.begin() != original.begin());
}

void testMoveConstructor() {
    Conversation original;

    original.append(Message(Role::User, "Hello"));
    original.append(Message(Role::Assistant, "Hi"));

    const Message* oldAddress = original.begin();

    Conversation moved(static_cast<Conversation&&>(original));

    assert(moved.size() == 2);
    assert(moved.begin() == oldAddress);

    assert(original.size() == 0);
    assert(original.begin() == nullptr);
}

void testGrowth() {
    Conversation conv;

    const Message* previousAddress = nullptr;
    int addressChanges = 0;

    for (int i = 0; i < 20; i++) {
        conv.append(Message(Role::User, to_string(i)));

        if (previousAddress != nullptr &&
            conv.begin() != previousAddress) {
            addressChanges++;
        }

        previousAddress = conv.begin();

        assert(conv.size() == static_cast<size_t>(i + 1));
        assert(conv.at(i).content() == to_string(i));
    }

    assert(addressChanges > 0);

    for (int i = 0; i < 20; i++) {
        assert(conv.at(i).content() == to_string(i));
    }
}

void testScannerCleanText() {
    SentinelScanner scanner("<|end_conversation|>");

    auto result = scanner.feed("Hello there");
    auto end = scanner.flush();

    assert(result.sentinel_found == false);
    assert(end.sentinel_found == false);

    assert(result.safe_text + end.safe_text == "Hello there");
}

void testScannerSplitSentinel() {
    string sentinel = "<|end_conversation|>";
    string text = "Goodbye." + sentinel;

    for (size_t split = 0; split <= text.size(); split++) {
        SentinelScanner scanner(sentinel);

        auto first = scanner.feed(text.substr(0, split));
        auto second = scanner.feed(text.substr(split));

        assert(first.sentinel_found || second.sentinel_found);

        assert(first.safe_text + second.safe_text == "Goodbye.");
    }
}

void testScannerFalseAlarm() {
    SentinelScanner scanner("<|end_conversation|>");

    string text = "Hello <|end_world|> goodbye";

    auto result = scanner.feed(text);
    auto end = scanner.flush();

    assert(result.sentinel_found == false);
    assert(end.sentinel_found == false);

    assert(result.safe_text + end.safe_text == text);
}

void testScannerLargeInput() {
    SentinelScanner scanner("<|end_conversation|>");

    string output;

    for (int i = 0; i < 100000; i++) {
        auto result = scanner.feed("a");

        assert(result.sentinel_found == false);

        output += result.safe_text;
    }

    auto end = scanner.flush();
    output += end.safe_text;

    assert(output.size() == 100000);
}

void testHarnessTurnLimit() {
    ofstream file("turn_limit_test.script");

    file << "role: assistant\n";
    file << "First response\n";
    file << "---\n";
    file << "role: assistant\n";
    file << "Second response\n";

    file.close();

    unique_ptr<ModelClient> model =
        make_unique<ScriptedModelClient>("turn_limit_test.script");

    HarnessConfig config;
    config.max_turns = 2;

    Harness harness(std::move(model), config);

    TestInput input({"Hello", "Again", "Extra"});
    TestOutput output;

    StopReason reason = harness.run(input, output);

    assert(reason.kind == StopReason::Kind::TurnLimit);
    assert(harness.conversation().size() == 4);

    remove("turn_limit_test.script");
}

void testHarnessSentinelHalt() {
    unique_ptr<ModelClient> model =
        make_unique<ScriptedModelClient>("scripts/greeting.script");

    HarnessConfig config;
    config.max_turns = 4;

    Harness harness(std::move(model), config);

    TestInput input({"Hello", "How are you?", "Goodbye", "unused"});
    TestOutput output;

   StopReason reason = harness.run(input, output);

cout << "Stop reason: " << static_cast<int>(reason.kind) << endl;
cout << "Detail: " << reason.detail << endl;
cout << "Output: " << output.output() << endl;

assert(reason.kind == StopReason::Kind::Sentinel);

    assert(output.output().find("<|end_conversation|>")
           == string::npos);

    assert(harness.conversation().at(
               harness.conversation().size() - 1
           ).content().find("<|end_conversation|>")
           != string::npos);
}

void testTranscriptRoundTrip() {
    ofstream file("replay_test.txt");

    file << "role: system\n";
    file << "Be concise.\n";
    file << "---\n";
    file << "role: user\n";
    file << "Hello\n";
    file << "---\n";
    file << "role: assistant\n";
    file << "Hi there!\n";

    file.close();

    ReplayModelClient replay("replay_test.txt");

    Conversation conv;
    conv.append(Message(Role::System, "Be concise."));
    conv.append(Message(Role::User, "Hello"));

    Message response = replay.generate(conv);

    assert(response.role() == Role::Assistant);
    assert(response.content() == "Hi there!");

    remove("replay_test.txt");
}

int main() {

    testEmptyConversation();
    testSystemMessageOrdering();
    testCopyConstructor();
    testMoveConstructor();
    testGrowth();

    testScannerCleanText();
    testScannerSplitSentinel();
    testScannerFalseAlarm();
    testScannerLargeInput();

    testHarnessTurnLimit();
    testHarnessSentinelHalt();
    testTranscriptRoundTrip();

    cout << "All 12 tests passed!" << endl;

    return 0;
}
