#if defined _tdialog_included
    #endinput
#endif

#include <open.mp>
#include <PawnPlus>
#include <sscanf2>

#if !defined TDIALOG_DIALOG_ID_BEGIN
    #define TDIALOG_DIALOG_ID_BEGIN (1234)
#endif

static enum
{
    TDIALOG_ID_FULL = TDIALOG_DIALOG_ID_BEGIN,
    TDIALOG_ID_NUMBER_INPUT,
    TDIALOG_ID_FLOAT_INPUT,
    TDIALOG_ID_STRING_INPUT,
    TDIALOG_ID_LISTITEM_TEXT,
    TDIALOG_ID_LISTITEM_INDEX,
    TDIALOG_ID_CONFIRMATION,
    TDIALOG_ID_OPTIONS_INDEX,
}

static const TDIALOG_DIALOG_ID_END = TDIALOG_ID_OPTIONS_INDEX; //always match to the bottom constant in the enum above

enum _:DIALOG_RESPONSE
{
    bool:DIALOG_RESPONSE_RESPONSE,
    DIALOG_RESPONSE_LISTITEM,
    DIALOG_RESPONSE_INPUTTEXT[128 + 1]
}

#if !defined FLOAT_NAN
	#define FLOAT_NAN	 (Float:0xFFFFFFFF)
#endif

#if !defined isnull
	#define isnull(%1) ((!(%1[0])) || (((%1[0]) == '\1') && (!(%1[1]))))
#endif

//Task variable so we don't leak memory
static Task:DialogTask[MAX_PLAYERS] = {INVALID_TASK, ...};

//Dynamic list we can use to keep track of arbitrary indexes or other dialog-related data
new List:DialogData[MAX_PLAYERS] = {INVALID_LIST, ...};

//Callback handlers so we can clean up across restarts
static CallbackHandler:PlayerConnectHandler = INVALID_CALLBACK_HANDLER;
static CallbackHandler:PlayerDisconnectHandler = INVALID_CALLBACK_HANDLER;
static CallbackHandler:DialogResponseHandler = INVALID_CALLBACK_HANDLER;

//Native wrapper to add a PawnPlus string variant of vanilla ShowPlayerDialog
native ShowPlayerDialog_s(playerid, dialogid, DIALOG_STYLE:style, ConstAmxString:title, ConstAmxString:body, const button1[], const button2[], AnyTag:...) = ShowPlayerDialog;

forward _pp@on_init@tdialogs();
public _pp@on_init@tdialogs()
{
    PlayerConnectHandler = pawn_register_callback("OnPlayerConnect", "TDialogs_PlayerConnect");
    PlayerDisconnectHandler = pawn_register_callback("OnPlayerDisconnect", "TDialogs_PlayerDisconnect");
    DialogResponseHandler = pawn_register_callback("OnDialogResponse", "TDialogs_DialogResponse");
}

forward _pp@on_exit@tdialogs();
public _pp@on_exit@tdialogs()
{
    pawn_unregister_callback(PlayerConnectHandler);
    pawn_unregister_callback(PlayerDisconnectHandler);
    pawn_unregister_callback(DialogResponseHandler);

    PlayerConnectHandler = INVALID_CALLBACK_HANDLER;
    PlayerDisconnectHandler = INVALID_CALLBACK_HANDLER;
}

forward TDialogs_PlayerConnect(playerid);
public TDialogs_PlayerConnect(playerid)
{
    ResetPlayerDialogVariables(playerid);

    DialogData[playerid] = list_new();
}

forward TDialogs_PlayerDisconnect(playerid, reason);
public TDialogs_PlayerDisconnect(playerid, reason)
{
    ResetPlayerDialogVariables(playerid);
}

stock Task:ShowAsyncDialog(playerid, DIALOG_STYLE:style, const title[], const body[], const button1[], const button2[] = "")
{
    pawn_call_native("ShowPlayerDialog", "iiissss", playerid, TDIALOG_ID_FULL, style, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncDialog_s(playerid, DIALOG_STYLE:style, String:title, String:body, const button1[], const button2[] = "")
{
    pawn_call_native("ShowPlayerDialog", "iiiSSss", playerid, TDIALOG_ID_FULL, style, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncNumberInputDialog(playerid, const title[], const body[], const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiissss", playerid, TDIALOG_ID_NUMBER_INPUT, DIALOG_STYLE_INPUT, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncNumberInputDialog_s(playerid, String:title, String:body, const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiiSSss", playerid, TDIALOG_ID_NUMBER_INPUT, DIALOG_STYLE_INPUT, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncFloatInputDialog(playerid, const title[], const body[], const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiissss", playerid, TDIALOG_ID_FLOAT_INPUT, DIALOG_STYLE_INPUT, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncFloatInputDialog_s(playerid, String:title, String:body, const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiiSSss", playerid, TDIALOG_ID_FLOAT_INPUT, DIALOG_STYLE_INPUT, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncStringInputDialog(playerid, const title[], const body[], const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiissss", playerid, TDIALOG_ID_STRING_INPUT, DIALOG_STYLE_INPUT, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncStringInputDialog_s(playerid, String:title, String:body, const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiiSSss", playerid, TDIALOG_ID_STRING_INPUT, DIALOG_STYLE_INPUT, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncPasswordDialog(playerid, const title[], const body[], const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiissss", playerid, TDIALOG_ID_STRING_INPUT, DIALOG_STYLE_PASSWORD, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncPasswordDialog_s(playerid, String:title, String:body, const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiiSSss", playerid, TDIALOG_ID_STRING_INPUT, DIALOG_STYLE_PASSWORD, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncListitemTextDialog(playerid, DIALOG_STYLE:style, const title[], const body[], const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiissss", playerid, TDIALOG_ID_LISTITEM_TEXT, style, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncListitemTextDialog_s(playerid, DIALOG_STYLE:style, String:title, String:body, const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiiSSss", playerid, TDIALOG_ID_LISTITEM_TEXT, style, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncListitemIndexDialog(playerid, DIALOG_STYLE:style, const title[], const body[], const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiissss", playerid, TDIALOG_ID_LISTITEM_INDEX, style, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncListitemIndexDialog_s(playerid, DIALOG_STYLE:style, String:title, String:body, const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiiSSss", playerid, TDIALOG_ID_LISTITEM_INDEX, style, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncConfirmationDialog(playerid, const title[], const body[], const button1[], const button2[] = "")
{
    pawn_call_native("ShowPlayerDialog", "iiissss", playerid, TDIALOG_ID_LISTITEM_INDEX, DIALOG_STYLE_MSGBOX, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncConfirmationDialog_s(playerid, ConstAmxStrring:title, ConstAmxStrring:body, const button1[], const button2[] = "")
{
    pawn_call_native("ShowPlayerDialog", "iiiSSss", playerid, TDIALOG_ID_LISTITEM_INDEX, DIALOG_STYLE_MSGBOX, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncEntityIndexDialog(playerid, DIALOG_STYLE:style, const title[], const body[], const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiissss", playerid, TDIALOG_ID_OPTIONS_INDEX, style, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

stock Task:ShowAsyncEntityIndexDialog_s(playerid, DIALOG_STYLE:style, String:title, String:body, const button1[], const button2[])
{
    pawn_call_native("ShowPlayerDialog", "iiiSSss", playerid, TDIALOG_ID_OPTIONS_INDEX, style, title, body, button1, button2);

    DeleteDialogTask(playerid);
    return DialogTask[playerid] = task_new();
}

forward TDialogs_DialogResponse(playerid, dialogid, response, listitem, inputtext[]);
public TDialogs_DialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if(dialogid < TDIALOG_DIALOG_ID_BEGIN || dialogid > TDIALOG_DIALOG_ID_END)
    {
        return false;
    }

    new Task:task = DialogTask[playerid];
    DialogTask[playerid] = INVALID_TASK;

    switch(dialogid)
    {
        case TDIALOG_ID_FULL:
        {
            new dialog_response[DIALOG_RESPONSE];
            dialog_response[DIALOG_RESPONSE_RESPONSE] = bool:response;
            dialog_response[DIALOG_RESPONSE_LISTITEM] = listitem;
            format(dialog_response[DIALOG_RESPONSE_INPUTTEXT], 128, inputtext);

            task_set_result_arr(task, dialog_response);
            return 1;
        }
        case TDIALOG_ID_NUMBER_INPUT:
        {
            if(!response)
            {
                task_set_result(task, cellmin);
                return 1;
            }

            new value;
            if(sscanf(inputtext, "i", value))
            {
                //show the dialog again when a player doesn't enter a number
                new DIALOG_STYLE:style, title[64], body[4096], button1[64], button2[64];
                GetPlayerDialogData(playerid, style, title, 64, body, 4096, button1, 64, button2, 64); //why was this function written like this
                task_bind(task, "_InputDialogErrorRedirect", "iissss", playerid, TDIALOG_ID_NUMBER_INPUT, title, body, button1, button2);
                return 1;
            }

            task_set_result(task, value);
            return 1;
        }
        case TDIALOG_ID_FLOAT_INPUT:
        {
            if(!response)
            {
                task_set_result(task, FLOAT_NAN);
                return 1;
            }

            new Float:value;
            if(sscanf(inputtext, "f", value))
            {
                //show the dialog again when a player doesn't enter a float
                new DIALOG_STYLE:style, title[64], body[4096], button1[12], button2[12];
                GetPlayerDialogData(playerid, style, title, 64, body, 4096, button1, 12, button2, 12); //why was this function written like this
                task_bind(task, "_InputDialogErrorRedirect", "iissss", playerid, TDIALOG_ID_FLOAT_INPUT, title, body, button1, button2);
                return 1;
            }

            task_set_result(task, value);
        }
        case TDIALOG_ID_STRING_INPUT, TDIALOG_ID_LISTITEM_TEXT:
        {
            if(!response)
            {
                task_set_result_str(task, "");
                return 1;
            }

            if(dialogid == TDIALOG_ID_STRING_INPUT && isnull(inputtext))
            {
                //show the dialog again when a player doesn't enter any text
                new DIALOG_STYLE:style, title[64], body[4096], button1[12], button2[12];
                GetPlayerDialogData(playerid, style, title, 64, body, 4096, button1, 12, button2, 12); //why was this function written like this
                task_bind(task, "_InputDialogErrorRedirect", "iissss", playerid, TDIALOG_ID_STRING_INPUT, title, body, button1, button2);
                return 1;
            }


            task_set_result_str(task, inputtext);
            return 1;
        }
        case TDIALOG_ID_LISTITEM_INDEX:
        {
            if(!response)
            {
                task_set_result(task, -1);
                return 1;
            }

            task_set_result(task, listitem);
        }
        case TDIALOG_ID_CONFIRMATION:
        {
            task_set_result(task, bool:response);
            return 1;
        }
        case TDIALOG_ID_OPTIONS_INDEX:
        {
            if(!response)
            {
                task_set_result(task, -1);
                return 1;
            }

            task_set_result(task, list_get(DialogData[playerid], listitem));
            return 1;
        }
    }
    return 0;
}

static ResetPlayerDialogVariables(playerid)
{
    DeleteDialogTask(playerid);

    if(list_valid(DialogData[playerid]))
    {
        //Using deep delete just in case people do weird things with the list
        list_delete_deep(DialogData[playerid]);
    }
}

static DeleteDialogTask(playerid)
{
    if(task_valid(DialogTask[playerid]))
    {
        task_delete(DialogTask[playerid]);
    }
    DialogTask[playerid] = INVALID_TASK;
}

forward _InputDialogErrorRedirect(playerid, dialogid, const title[], const body[], const button1[], const button2[]);
public _InputDialogErrorRedirect(playerid, dialogid, const title[], const body[], const button1[], const button2[])
{
    switch(dialogid)
    {
        case TDIALOG_ID_NUMBER_INPUT: return task_await(ShowAsyncNumberInputDialog(playerid, title, body, button1, button2));
        case TDIALOG_ID_FLOAT_INPUT: return task_await(ShowAsyncFloatInputDialog(playerid, title, body, button1, button2));
        case TDIALOG_ID_STRING_INPUT: return task_await(ShowAsyncStringInputDialog(playerid, title, body, button1, button2));
    }
    return wait_ms(1);
}