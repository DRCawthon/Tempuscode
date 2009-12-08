#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <string>
#include <list>
#include "constants.h"
#include "prog.h"

using namespace std;

const int MAX_MAIL_ATTACHMENTS = 5;
const int MAIL_COST_MULTIPLIER = 30;

class CEditor {
public:
	CEditor(struct descriptor_data *d, int max);
    virtual ~CEditor(void) { }

	// Command Processor
	void Process(char *inStr);
	void SendStartupMessage(void);
    void SendPrompt(void);

	virtual bool IsEditing(char *inStr) =0;
    void Finish(bool save);

protected:
    CEditor() { }

    // These are different between subclasses
    virtual bool PerformCommand(char cmd, char *args);
    virtual void Finalize(const char *text) = 0;
    virtual void Cancel(void);
    virtual void DisplayBuffer(unsigned int start_line = 1, int line_count = -1);
    virtual void SendModalHelp(void); // send mode-specific command help

	void ImportText(const char *text);	// Run from contructor, imports *d->str
	void SendMessage(const char *message);	// Wrapper for sendtochar
	void UndoChanges(void);

	// The descriptor of the player we are dealin with.
	struct descriptor_data *desc;

	// the text
    list<string> origText;
    list<string> theText;
	unsigned int curSize;
	unsigned int maxSize;
    bool wrap;

private:
	void ProcessHelp(char *inStr);
	void Help(char *inStr);		// Open refrigerator?
	void UpdateSize(void);
	bool Wrap(void);			// Wordwrap
	bool Full(char *inStr = NULL);
	// Below: The internal text manip commands
	void Append(char *inStr);	// Standard appending lines.
	bool Insert(unsigned int line, char *inStr);	// Insert a line
	bool ReplaceLine(unsigned int line, char *inStr);	// Replace a line
    bool MoveLines(unsigned int start_line,
                   unsigned int end_line,
                   unsigned int destination);
	bool Find(char *args);	// Text search
	bool Substitute(char *args);	// Text search and replace.
    bool Remove(unsigned int start_line, unsigned int finish_line);
	bool Clear(void);			// Wipe the text and start over.
};

class CTextEditor : public CEditor {
public:
    CTextEditor(descriptor_data *desc, char **target, int max);
    virtual ~CTextEditor(void);

    virtual bool IsEditing(char *inStr __attribute__ ((unused)))
    {
		return (inStr == *target);
	}

protected:
    CTextEditor(void);

	// The destination char **
	char **target;

    virtual bool PerformCommand(char cmd, char *args);
    virtual void Cancel(void);
    virtual void Finalize(const char *text);
};

class CMailEditor : public CEditor {
public:
    CMailEditor(descriptor_data *desc,
                mail_recipient_data *recipients);
    virtual ~CMailEditor(void);

    virtual bool IsEditing(char *inStr __attribute__ ((unused)))
    {
        return false;
    }

protected:
    CMailEditor(void);

    virtual void DisplayBuffer(unsigned int start_line = 1, int line_count = -1);
    virtual bool PerformCommand(char cmd, char *args);
    virtual void Finalize(const char *text);
    virtual void Cancel(void);
    virtual void SendModalHelp(void);

	void ListRecipients(void);
	void ListAttachments(void);
	void AddRecipient(char *name);
	void RemRecipient(char *name);
	void AddAttachment(char *obj);
    void ReturnAttachments(void);

    mail_recipient_data *mail_to;
    struct obj_data *obj_list;
    int num_attachments;

};

class CProgEditor : public CEditor {
public:
    CProgEditor(descriptor_data *desc, thing *o, prog_evt_type t);
    virtual ~CProgEditor(void);

    virtual bool IsEditing(char *inStr)
    {
        return (inStr == prog_get_text(owner, owner_type));
	}

protected:
    CProgEditor(void);

	// The destination char **
	thing *owner;
    prog_evt_type owner_type;

    virtual bool PerformCommand(char cmd, char *args);
    virtual void Finalize(const char *text);
};

class CBoardEditor : public CEditor {
public:
    CBoardEditor(descriptor_data *desc, const char *b_name, int id, const char *subject, const char *body);
    virtual ~CBoardEditor(void);

    virtual bool IsEditing(char *inStr __attribute__ ((unused)))
    {
        return false;
	}

protected:
    CBoardEditor(void);

    int idnum;
    char *board_name;
    char *subject;

    virtual bool PerformCommand(char cmd, char *args);
    virtual void Finalize(const char *text);
};

class CPollEditor : public CEditor {
public:
    CPollEditor(descriptor_data *desc, const char *header);
    virtual ~CPollEditor(void);

    virtual bool IsEditing(char *inStr __attribute__ ((unused)))
    {
        return false;
	}

protected:
    CPollEditor(void);

    char *header;

    virtual void Finalize(const char *text);
};

class CFileEditor : public CEditor {
public:
    CFileEditor(descriptor_data *desc, const char *fname);
    virtual ~CFileEditor(void);

    virtual bool IsEditing(char *inStr __attribute__ ((unused))) {
        return false;
    }

    bool loaded;
    string fname;

protected:
    CFileEditor(void);

    virtual bool PerformCommand(char cmd, char *args);
    virtual void Finalize(const char *text);
};

// Interfaces to the editor from the outside world.
void start_editing_text(struct descriptor_data *d,
                        char **target,
                        int max = MAX_STRING_LENGTH);

void start_editing_mail(struct descriptor_data *d,
                        mail_recipient_data *recipients);

void start_editing_prog(struct descriptor_data *d,
                        thing *owner,
                        prog_evt_type owner_type);
void start_editing_board(struct descriptor_data *d,
                         const char *board,
                         int idnum,
                         const char *subject,
                         const char *body);
void start_editing_poll(struct descriptor_data *d,
                        const char *header);
void start_editing_file(struct descriptor_data *d,
                        const char *fname);
#endif
