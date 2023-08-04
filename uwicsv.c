#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "uwi.h"

#define MAX_COL_COUNT 10000
#define MAX_ROW_COUNT 100000
#define MAX_CHR_COUNT 1000

// Data Type for storing data of a Column
typedef struct csvColumn
{
    int    currentRow;          // Current row of this column
    double real[MAX_ROW_COUNT]; // Real valued Column data
    double imag[MAX_ROW_COUNT]; // Imaginary Column data
} column;

// Data Type for storing Columns
struct csvData
{
    FILE   *csvFile;               // Defines output file
    char   *ids[MAX_COL_COUNT];    // Column IDs
    column columns[MAX_COL_COUNT]; // All columns in the file
    int    numCols;                // Number of Columns in the file
    int    numRows;                // Number of Rows in the file
    short  isDigital:1;            // Digital wave form Flag
    short  isComplex:1;            // Complex wave form Flag
    char   sweepVar[5];            // Name of Sweep Variable
    double sweep[MAX_ROW_COUNT];   //  Sweep variable Column
};
static struct csvData data;        // Static instance for storing waveform

// Convert logic integer waveform value to string representation: {0, 1, x, z}
char* l2s(int val)
{
    if (val == 0)
        return "0";
    else if (val == 1)
        return "1";
    else if (val == 2)
        return "x";
    else if (val == 3)
        return "z";
    else
        return "";
}

// Open CSV File
uwi_StreamHandle csvOpen(const struct uwi_Setup* setup)
{
    char name[MAX_CHR_COUNT];
    int nameLen  = MAX_CHR_COUNT - 10;
    data.csvFile = NULL;
    data.numCols = 0;

    snprintf(name, nameLen, "%s/%s", setup->rawDir, setup->fileName);
    strcat(name, ".csv");

    if(setup->analysis == UWI_TRAN)
        strcpy(data.sweepVar, "time");
    else if(setup->analysis == UWI_AC)
        strcpy(data.sweepVar, "freq");
    else
        strcpy(data.sweepVar, "sweep");

    data.csvFile = fopen(name, "w+");
    return data.csvFile;
}

// Close CSV File
int csvClose(uwi_StreamHandle stream)
{
    fclose((FILE*) stream);

    for(int i = 0; i < data.numCols; ++i)
        free(data.ids[i]);

    return 0;
}

// Define Waveform in CSV Format
uwi_WfHandle csvDefine( uwi_StreamHandle stream
                      , const struct uwi_WfDefinition* wfDef )
{
    int it;
    int index, lenID = 0;

    if (wfDef->wfName)
        lenID = strlen(wfDef->wfName);

    if (wfDef->scopeName && wfDef->scopeName[0] != 0) {
        for (it = 0; wfDef->scopeName[it] != 0; ++it)
            lenID += 1 + strlen(wfDef->scopeName[it]);
    }
    if (lenID && data.numCols < MAX_COL_COUNT)
        data.ids[data.numCols] = (char *)malloc(lenID + 1);
    else
        return NULL;

    if (wfDef->scopeName && wfDef->scopeName[0] != 0) {
        data.ids[data.numCols][0] = '\0';
        for (it = 0; wfDef->scopeName[it] != 0; ++it ) {
            strcat(data.ids[data.numCols], wfDef->scopeName[it]);
            strcat(data.ids[data.numCols], "_");
        }
        strcat(data.ids[data.numCols], wfDef->wfName);
    }
    else
        strcpy(data.ids[data.numCols], wfDef->wfName);

    index = ++data.numCols;

    data.columns[index].currentRow = 0;
    memset(data.columns[index].real, 0.0, sizeof data.columns[index].real);
    memset(data.columns[index].imag, 0.0, sizeof data.columns[index].imag);

    return (uwi_WfHandle)(size_t)index;
}

// Flush waveform to csv file
int csvFlush(uwi_StreamHandle stream)
{
    FILE *fp = (FILE*) stream;
    column col;

    fprintf(fp, "%s,", data.sweepVar);

    for (int i = 0; i < data.numCols; ++i) {
        fprintf(fp, "%s", data.ids[i]);
        if (i < data.numCols - 1)
            fprintf(fp, ",");
        else
            fprintf(fp, "\n");
    }

    for (int r = 0; r < data.numRows; ++r) {
        fprintf(fp, "%lf", data.sweep[r]);
        if (data.isComplex)
            fprintf(fp, ":+0.0");
        fprintf(fp, ",");
        for (int c = 1; c < data.numCols + 1; ++c) {
            col = data.columns[c];
            if (data.isDigital) {
                char* val = l2s((int) col.real[r]);
                fprintf(fp, "%s", val);
            }
            else {
                fprintf(fp, "%lf", col.real[r]);
                if (data.isComplex)
                    fprintf(fp, ":+%lf", col.imag[r]);
            }
            if (c < data.numCols)
                fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }

    data.numRows = 0;
    for (int c = 0; c < data.numCols; ++c)
        data.columns[c].currentRow = 0;
    return 0;
}

// Add Real valued Analog data point
int csvAddAWfPoint( uwi_StreamHandle stream
                  , uwi_WfHandle wfHandle
                  , double real
                  , double time )
{
    int colIdx = (size_t)(wfHandle);
    int idx = data.columns[colIdx].currentRow++;
     
    if (idx < MAX_ROW_COUNT) {
        data.columns[colIdx].real[idx] = real;
        data.sweep[idx]                = time;
        data.isDigital                 = 0;
        data.isComplex                 = 0;
        data.numRows                   = data.numRows < (idx + 1) ? (idx + 1) : data.numRows;
    }
    else
        printf("Exceeded the maximum data point that can be stored \n");

    return 0;
}

// Add Complex valued Analog data point
int csvAddAWfComplexPoint( uwi_StreamHandle stream
                         , uwi_WfHandle wfHandle
                         , double real
                         , double imag
                         , double freq )
{
    int colIdx = (size_t)(wfHandle);
    int idx = data.columns[colIdx].currentRow++;

    if (idx < MAX_ROW_COUNT) {
        data.columns[colIdx].real[idx] = real;
        data.columns[colIdx].imag[idx] = imag;
        data.sweep[idx]                = freq;
        data.isDigital                 = 0;
        data.isComplex                 = 1;
        data.numRows                   = data.numRows < (idx + 1) ? (idx + 1) : data.numRows;
    }
    else
        printf("Exceeded the maximum data point that can be stored \n");
    return 0;
}

// Add Digital data point
int csvAddDWfPoint( uwi_StreamHandle stream
                  , uwi_WfHandle wfHandle
                  , enum uwi_Logic value
                  , double time )
{
    int colIdx = (size_t)(wfHandle);
    int idx = data.columns[colIdx].currentRow++;

    if (idx < MAX_ROW_COUNT) {
        data.columns[colIdx].real[idx] = (double) value;
        data.sweep[idx]                = time;
        data.isDigital                 = 1;
        data.isComplex                 = 0;
        data.numRows                   = data.numRows < (idx + 1) ? (idx + 1) : data.numRows;
    }
    else
        printf("Exceeded the maximum data point that can be stored \n");
    return 0;
}

// Register CSV
struct uwi_WfIntDef* uwi_register()
{
    static struct uwi_WfIntDef wfIntDef;
    wfIntDef.open               = csvOpen;
    wfIntDef.defineWf           = csvDefine;
    wfIntDef.addAWfPoint        = csvAddAWfPoint;
    wfIntDef.addAWfComplexPoint = csvAddAWfComplexPoint;
    wfIntDef.addDWfPoint        = csvAddDWfPoint;
    wfIntDef.flush              = csvFlush;
    wfIntDef.close              = csvClose;
    wfIntDef.getErrMsg          = NULL;
    wfIntDef.resetXCoord        = NULL;
    wfIntDef.format             = "CSV";
    return &wfIntDef;
}
