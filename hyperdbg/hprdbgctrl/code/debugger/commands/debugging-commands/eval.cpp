/**
 * @file eval.cpp
 * @author Sina Karvandi (sina@rayanfam.com)
 * @brief eval (?) command
 * @details
 * @version 0.1
 * @date 2021-02-05
 *
 * @copyright This project is released under the GNU Public License v3.
 *
 */
#include "..\hprdbgctrl\pch.h"

using namespace std;

//
// Global Variables
//
extern BOOLEAN g_IsSerialConnectedToRemoteDebuggee;

/**
 * @brief help of ? command
 *
 * @return VOID
 */
VOID
CommandEvalHelp()
{
    ShowMessages("? : evaluate and execute expressions in debuggee.\n\n");
    ShowMessages("syntax : \t? [expression]\n");
    ShowMessages("\t\te.g : ? print(dq(poi(@rcx)));\n");
    ShowMessages("\t\te.g : ? json(dq(poi(@rcx)));\n");
}

/**
 * @brief Check test-cases for script-engine
 *
 * @return BOOLEAN
 */
BOOLEAN
CommandEvalCheckTestcase()
{
    string                   Line;
    BOOLEAN                  IsOpened      = FALSE;
    UINT64                   ExpectedValue = 0;
    BOOLEAN                  ExpectError   = FALSE;
    string                   Expr          = "";
    std::vector<std::string> TestFiles;

    try
    {
        TestFiles = ListDirectory(SCRIPT_ENGINE_TEST_CASES_DIRECTORY, "*.txt");
    }
    catch (const std::exception &)
    {
        ShowMessages("err, test-cases not found, make sure to run test-environment.py "
                     "before running test cases\n");
        return FALSE;
    }

    for (auto item : TestFiles)
    {
        //
        // Read the test-case file for script-engine
        //
        ifstream File(item.c_str());

        if (File.is_open())
        {
            IsOpened = TRUE;

            //
            // File name
            //
            ShowMessages("Running from : %s\n\n", item.c_str());

            while (std::getline(File, Line))
            {
                //
                // Test case number
                //
                ShowMessages("Test-case number : %s\n", Line.c_str());

                //
                // Test-case statement
                //
                if (!std::getline(File, Line))
                {
                    return FALSE;
                }

                Expr = Line;
                ShowMessages("Statement : %s\n", Line.c_str());

                //
                // Test-case result
                //
                if (!std::getline(File, Line))
                {
                    return FALSE;
                }

                if (!Line.compare("$error$"))
                {
                    //
                    // It's an $error$ statement
                    //
                    ShowMessages("Expected result : %s\n", Line.c_str());

                    ExpectError   = TRUE;
                    ExpectedValue = NULL;
                }
                else if (!ConvertStringToUInt64(Line, &ExpectedValue))
                {
                    ShowMessages("err, the expected results are in incorrect format\n");
                    return FALSE;
                }
                else
                {
                    //
                    // It's a value expected statement
                    //
                    ExpectError = FALSE;
                    ShowMessages("Expected result : %llx\n", ExpectedValue);
                }

                //
                // Call wrapper for testing statements
                //
                Expr.append(" ");

                //
                // Test results
                //
                ShowMessages("Test result : %s\n", ScriptAutomaticStatementsTestWrapper(Expr, ExpectedValue, ExpectError) ? "Passed" : "Failed");

                //
                // Test-case end
                //
                if (!std::getline(File, Line))
                {
                    return FALSE;
                }

                //
                // Check end case
                //
                if (Line.compare("$end$"))
                {
                    //
                    // err, we'd expect a $end$ at this situation
                    //
                    return FALSE;
                }

                ShowMessages("\n------------------------------------------------------------\n\n");
            }

            File.close();
        }
    }

    if (!IsOpened)
    {
        ShowMessages("err, could not find files for script engine test-cases\n");
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief handler of ? command
 *
 * @param SplittedCommand
 * @param Command
 * @return VOID
 */
VOID
CommandEval(vector<string> SplittedCommand, string Command)
{
    PVOID  CodeBuffer;
    UINT64 BufferAddress;
    UINT32 BufferLength;
    UINT32 Pointer;

    if (SplittedCommand.size() == 1)
    {
        ShowMessages("incorrect use of '?'\n\n");
        CommandEvalHelp();
        return;
    }

    //
    // Trim the command
    //
    Trim(Command);

    //
    // Remove first command from it
    //
    Command.erase(0, SplittedCommand.at(0).size());

    //
    // Trim it again
    //
    Trim(Command);

    //
    // Check if it's a test-case check or not
    //
    if (!Command.compare("test"))
    {
        //
        // It's a test-case checker
        //
        if (!CommandEvalCheckTestcase())
        {
            ShowMessages("testing script engine test-cases was not successful!\n");
        }

        return;
    }

    //
    // TODO: end of string must have a whitspace. fix it.
    //
    Command.append(" ");
    // Expr = " x = 4 >> 1; ";

    if (g_IsSerialConnectedToRemoteDebuggee)
    {
        //
        // Send over serial
        //

        //
        // Run script engine handler
        //
        CodeBuffer = ScriptEngineParseWrapper((char *)Command.c_str(), TRUE);

        if (CodeBuffer == NULL)
        {
            //
            // return to show that this item contains an script
            //
            return;
        }

        //
        // Print symbols (test)
        //
        // PrintSymbolBufferWrapper(CodeBuffer);

        //
        // Set the buffer and length
        //
        BufferAddress = ScriptEngineWrapperGetHead(CodeBuffer);
        BufferLength  = ScriptEngineWrapperGetSize(CodeBuffer);
        Pointer       = ScriptEngineWrapperGetPointer(CodeBuffer);

        //
        // Send it to the remote debuggee
        //
        KdSendScriptPacketToDebuggee(BufferAddress, BufferLength, Pointer, FALSE);

        //
        // Remove the buffer of script engine interpreted code
        //
        ScriptEngineWrapperRemoveSymbolBuffer(CodeBuffer);
    }
    else
    {
        //
        // It's a test
        //
        ShowMessages("this command should not be used while you're in VMI-Mode or not in debugger-mode, "
                     "the results that you see is a simulated result for TESTING script-engine "
                     "and is not based on the status of your system. You can use this command, "
                     "ONLY in debugger-mode\n\n");

        ShowMessages("test expression : %s \n", Command.c_str());
        ScriptEngineWrapperTestParser(Command);
    }
}
