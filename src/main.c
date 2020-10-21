//Evan Von Oehseen
//eavb2018@mymail.pomona.edu
//10-20-2020

#include <stdlib.h>
#include <stdint.h>

int numberGen();
void preparePins();
int comparePatterns(int a[], int b[], int rounds);
void blink(int colorPin);
void flash(int colorPin);
int waitForInput();
void delay();
void displayPattern(int pattern[], int round);
void pinWrite(int pinNum, int value);
int pinRead(int pinNum);
void doubleBlink(int colorPin1, int colorPin2);

int DUR = 100;
int grInput = 20;
int rdInput = 21;
int grOutput = 22;
int rdOutput = 23;

volatile uint32_t *GPIO_INPUT_EN = (uint32_t *)0x10012004U;
volatile uint32_t *GPIO_OUTPUT_EN = (uint32_t *)0x10012008U;

volatile uint32_t *GPIO_INPUT_VAL = (uint32_t *)0x10012000U;
volatile uint32_t *GPIO_OUTPUT_VAL = (uint32_t *)0x1001200CU;

int main(void)
{
    preparePins();

    doubleBlink(rdOutput, grOutput);
    delay(DUR * 10);

    int step = 0;
    int stepMax = 0;
    int sequence[12];
    int userInput[12];

    while (1)
    {
        if (stepMax == 0)
        {//first two steps after reset
            sequence[0] = numberGen();
            sequence[1] = numberGen();
            stepMax = 2;

            displayPattern(sequence, stepMax);
        }
        else if (stepMax > 12)
        {//user made it to 12! Time to reset
            flash(grOutput);
            flash(grOutput);
            flash(grOutput);

            stepMax = 0;
            step = 0;

            delay(DUR * 10);
            doubleBlink(rdOutput, grOutput);
        }
        else if (step == 0)
        {
            while (step < stepMax)
            {
                int nextInput = waitForInput();
                userInput[step] = nextInput;
                step++;
            }
        }
        else
        {
            delay(DUR * 5);
            if (comparePatterns(sequence, userInput, stepMax))
            { // user got it right! Adding another to the sequence
                stepMax++;
                step = 0;
                blink(grOutput);
                sequence[stepMax - 1] = numberGen();
                displayPattern(sequence, stepMax);
            }
            else
            { //resets the game due to an improper sequence
                stepMax = 0;
                step = 0;
                blink(rdOutput);
                delay(DUR * 10);
                doubleBlink(rdOutput, grOutput);
            }
        }
    }
}

int numberGen()
{
    return rand() % 2;
}

void preparePins()
{
    //set or disable input mode for relevant pins
    *GPIO_INPUT_EN |= (1 << grInput);
    *GPIO_INPUT_EN |= (1 << rdInput);
    // *GPIO_INPUT_EN &= ~(1 << grOutput);
    // *GPIO_INPUT_EN &= ~(1 << rdOutput);
    //set or disable output mode for relevant pins
    *GPIO_OUTPUT_EN |= (1 << grOutput);
    *GPIO_OUTPUT_EN |= (1 << rdOutput);
    // *GPIO_OUTPUT_EN &= ~(1 << grInput);
    // *GPIO_OUTPUT_EN &= ~(1 << rdInput);
}

int comparePatterns(int a[], int b[], int rounds)
{
    for (int i = 0; i < rounds; i++)
    {
        if (a[i] != b[i])
        {
            return 0;
        }
    }
    return 1;
}

void blink(int colorPin)
{
    for (int i = 0; i < 5; i++)
    {
        pinWrite(colorPin, 1);
        delay(DUR);
        pinWrite(colorPin, 0);
        delay(DUR);
    }
}

void doubleBlink(int colorPin1, int colorPin2)
{
    for (int i = 0; i < 10; i++)
    {
        pinWrite(colorPin1, 1);
        pinWrite(colorPin2, 0);
        delay(DUR);
        pinWrite(colorPin1, 0);
        pinWrite(colorPin2, 1);
        delay(DUR);
    }
    pinWrite(colorPin2, 0);
}

void flash(int colorPin)
{
    delay(DUR * 6);
    pinWrite(colorPin, 1);
    delay(DUR * 6);
    pinWrite(colorPin, 0);
}

int waitForInput()
{
    while (!((*GPIO_INPUT_VAL >> rdInput) & 0b1) && !((*GPIO_INPUT_VAL >> grInput) & 0b1));
    int input = pinRead(rdInput);

    while (((*GPIO_INPUT_VAL >> rdInput) & 0b1) || ((*GPIO_INPUT_VAL >> grInput) & 0b1));

    return input;
}

void delay(int ms)
{
    volatile uint64_t *mtime = (uint64_t *)0x0200bff8;
    uint64_t doneTime = *mtime + (ms * 32768) / 1000;
    while (*mtime < doneTime); // wait until time is reached
}

void displayPattern(int pattern[], int round)
{
    delay(DUR*10);

    for (int i = 0; i < round; i++)
    {
        flash(grOutput + pattern[i]);
    }
}

void pinWrite(int pinNum, int value)
{
    if (value)
    {
        *GPIO_OUTPUT_VAL |= (1 << pinNum);
    }
    else
    {
        *GPIO_OUTPUT_VAL &= ~(1 << pinNum);
    }
}

int pinRead(int pinNum)
{
    return (*GPIO_INPUT_VAL >> pinNum) & 0b1;
}