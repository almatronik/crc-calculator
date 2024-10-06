#include <stdio.h>
#include <stdlib.h>

#define byte_size 8
#define min_msg_bytes 1
#define max_msg_bytes 14

/*********************************************************************\
|*          <===        DECLARATIONS     ===>                        *|
\*********************************************************************/

int tmp_input[max_msg_bytes] = {0};

void buffer_flushing();
int get_polynomial_length();
void get_polynomial(int *polynomial_int_array, int polynomial_length);
int get_input(int *tmp_input_array);
int calc_msg_len(int bytes);
int binary_conversion(int *msg_whole, int *msg_arr, int bytes_lenght);
void binary_conversion_per_byte(int *byte_array, int temp);
int add_padding(int *msg_whole, int bits, int padding);
void transfer_array(int *array1, int *array2, int start1, int start2, int end1, int end2);
void print_array(int *array, int start, int end, char top_text[], char bottom_text[]);
void print_ascii(int *tmp_input_array, int *msg_array, int msg_byte_length);
void print_char(char c, int repeats);
void crc_check(int *transmission_array, int *polynomial_array, int *checksum_array, int transmission_length, int msg_length, int padding_length, int polynomial_length);
void initialize_array(int *array, int length);
int validate_result(int *transmission_array, int transmission_length);


/*********************************************************************\
|*          <===        MAIN STARTS HERE      ===>                   *|
\*********************************************************************/

int main(void)
{
    int polynomial_len = get_polynomial_length();

    int polynomial[polynomial_len];
    initialize_array(polynomial, polynomial_len);
    get_polynomial(polynomial, polynomial_len);

    int msg_byte_len = get_input(tmp_input); //get_input function returns the byte counter
    int msg_len = msg_byte_len * byte_size; //msg_len in bits without the padding


    int msg[msg_byte_len]; //Message in ASCII form
    initialize_array(msg, msg_byte_len);

    transfer_array(tmp_input, msg, 0, 0, msg_byte_len, msg_byte_len);
    print_ascii(tmp_input, msg, msg_byte_len);





    polynomial_len = sizeof(polynomial) / sizeof(polynomial[0]);
    int padding = polynomial_len - 1;

    int transmission_len = msg_len + padding;
    int transmission[transmission_len];
    initialize_array(transmission, transmission_len);
    int msg_in_bits[transmission_len]; //Message in binary
    initialize_array(msg_in_bits, transmission_len);
    int bit_counter = binary_conversion(msg_in_bits, msg, msg_byte_len);
    bit_counter = add_padding(msg_in_bits, bit_counter, padding);

    printf("Length of transmission: %d bits (%d bits message, %d bits padding)\n", transmission_len, msg_len, padding);

    int checksum[padding];
    initialize_array(checksum, padding);

    transfer_array(msg_in_bits, transmission, 0, 0, transmission_len, transmission_len);
    print_array(msg_in_bits, 0, bit_counter, "\nTransmission (original binary message + padding):\n", "");
    print_array(polynomial, 0, polynomial_len, "\n    Polynomial: ", "\n\n");

    crc_check(transmission, polynomial, checksum, transmission_len, msg_len, padding, polynomial_len); //CRC Computing

    transfer_array(transmission, checksum, msg_len, 0, transmission_len, padding);
    transfer_array(transmission, transmission, msg_len, msg_len, transmission_len, transmission_len);
    transfer_array(msg_in_bits, transmission, 0, 0, msg_len, msg_len);
    print_array(checksum, 0, padding, "\nThe checksum is: ", "\n\n");
    print_array(transmission, 0, transmission_len, "Transmission (original binary message + checksum):\n", "\nThe transmission will now be verified against the same polynomial:\n\n");

    crc_check(transmission, polynomial, checksum, transmission_len, msg_len, padding, polynomial_len); //CRC Verification

    print_char('=', 20);
    printf("\nRESULT:\n");
    int result = validate_result(transmission, transmission_len);
    (result == -1) ? printf("Validation failed!\n") : printf("Validation confirmed!\n");
    print_char('=', 20);



    return 0;
}

/*********************************************************************\
|*          <===        MAIN ENDS HERE      ===>                     *|
\*********************************************************************/

/*********************************************************************\
|*          <===        CRC FUNCTION STARTS HERE      ===>           *|
\*********************************************************************/

void crc_check(int *transmission_array, int *polynomial_array, int *checksum_array, int transmission_length, int msg_length, int padding_length, int polynomial_length)
{
	int count = 0;
	int dividend[transmission_length];
	transfer_array(transmission_array, dividend, 0, 0, transmission_length, transmission_length);
	int result[transmission_length];
	initialize_array(result, transmission_length);
	out_of_while:
	while (count < (transmission_length - padding_length))
	{
		printf("#%d\n", count);
		for (int i = count, j = 0; i < transmission_length + count, j < (padding_length+1); i++)
        {
            if (dividend[count] == 0)
            {
                printf("Leading %d: >> 1\n", dividend[count]);
                count++;
                //continue;
                goto out_of_while; //Breaks out of the while loop (11 lines above)
            }
            else if (dividend[count] != 0)
            {
                if (dividend[i] == polynomial_array[j])
                {
                    result[i] = 0;
                }
                else
                {
                    result[i] = 1;
                }
                j++;
            }
        }
        transfer_array(dividend, result, (count+polynomial_length), (count+polynomial_length), transmission_length, transmission_length);

        print_array(dividend, 0, transmission_length, "", " dividend\n");
        print_char(' ', count);
        print_array(polynomial_array, 0, polynomial_length, "", "");
        print_char(' ', msg_length-count);
        printf("polynomial\n", count);
        print_array(result, 0, transmission_length, "", " result\n");
        print_char('-', 79);
        printf("\n");
        transfer_array(result, dividend, 0, 0, transmission_length, transmission_length);

        count++;
    }
    transfer_array(result, transmission_array, 0, 0, transmission_length, transmission_length);
    transfer_array(result, checksum_array, msg_length, 0, transmission_length, padding_length);
}

/*********************************************************************\
|*          <===        CRC FUNCTION ENDS HERE      ===>             *|
\*********************************************************************/

int get_input(int *tmp_input_array)
{
    int error, count, i;
    do
    {
        i = 0, count = 0, error = 0;
        char ch;
        printf("\nType your message (Size range %d-%d bytes): ", min_msg_bytes, max_msg_bytes);
        while ((ch = getchar()) && ch != '\n')
        {
            *(tmp_input_array + i++) = ch;
            count++;
        }
        if (count > max_msg_bytes || count < min_msg_bytes)
        {
            printf("The message is too big or too small: Try again!\n");
            count = 0, error = 1;
        }
    }
    while (error == 1);
    return count;
}

void buffer_flushing()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        ;
    }
}

int get_polynomial_length()
{
    int polynomial_length = 0;
    do
    {
        printf("Provide the length of your polynomial in bits. Minimum 4 bits: ");
        scanf("%d", &polynomial_length);
        buffer_flushing();
    }
    while (polynomial_length < 4);
    return polynomial_length;
}
void get_polynomial(int *polynomial_int_array, int polynomial_length)
{
    char temp_char_array[polynomial_length+1];
    printf("\nType the polynomial.\nHint=1100010110011001\n");
    fgets(temp_char_array, polynomial_length+1, stdin);
    buffer_flushing();
    for (int i = 0; i < polynomial_length; i++)
    {
        *(polynomial_int_array + i) = temp_char_array[i] - '0';
    }
}

void print_array(int *array, int start, int end, char top_text[], char bottom_text[])
{
    printf("%s", top_text);
    for (int i = start; i < end; i++)
    {
        printf("%d", *(array + i));
    }
    printf("%s", bottom_text);
}

void print_char(char c, int repeats)
{
    for (int i = 0; i < repeats; i++)
    {
        printf("%c", c);
    }
}

void transfer_array(int *array1, int *array2, int start1, int start2, int end1, int end2)
{
    for (int i = start1, j = start2; i < end1, j < end2; i++, j++)
    {
        *(array2 + j) = *(array1 + i);
    }
}

void print_ascii(int *tmp_input_array, int *msg_array, int msg_byte_length)
{
    for (int i = 0; i < msg_byte_length; i++)
    {
        printf("%c = %d\n", *(msg_array + i), *(msg_array + i));
    }
}

void binary_conversion_per_byte(int *byte_array, int temp)
{
	for (int j = byte_size - 1; j >= 0; j--)
    {
        *(byte_array + j) = temp % 2;
        temp = temp / 2;
    }
}

int binary_conversion(int *msg_whole, int *msg_arr, int bytes_lenght)
{
	int bits = 0;
    for (int i = 0; i < bytes_lenght; i++)
    {
        int bin_conv[byte_size];
        int tmp = *(msg_arr + i);

        binary_conversion_per_byte(bin_conv, tmp);
        print_array(bin_conv, 0, byte_size, "", "\n");

        for (int j = 0; j < byte_size; j++)
        {
            *(msg_whole + bits++) = bin_conv[j];
        }
    }
    printf("\n");

	return bits;
}

int add_padding(int *msg_whole, int bits, int padding)
{
	for (int i = 0; i < padding; i++)
    {
        *(msg_whole + bits++) = 0;
    }
    return bits;
}

void initialize_array(int *array, int length)
{
    for (int i = 0; i < length; i++)
    {
        *(array + i) = 0;
    }
}

int validate_result(int *transmission_array, int transmission_length)
{
    for (int i = 0; i < transmission_length; i++)
    {
        if (*(transmission_array + i) != 0)
        {
            return -1;
        }
        return 0;
    }
}
