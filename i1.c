// i1.c
//Moss Amann
#include <stdio.h>    // for I/O functions
#include <stdlib.h>   // for exit()
#include <time.h>     // for time functions

FILE *infile;
short r[8], mem[65536], offset6, imm5, imm9, pcoffset9, pcoffset11, 
      regsave1, regsave2;
unsigned short ir, pc, opcode, code, dr, sr, sr1, sr2, baser, bit5, bit11,
               trapvec, eopcode, n, z, c, v;
char letter;

void setnz(short r)
{
   n = z = 0;
   if (r < 0)    // is result negative?
      n = 1;     // set n flag
   else
   if (r == 0)   // is result zero?
      z = 1;     // set z flag
}

void setcv(short sum, short x, short y)
{
   v = c = 0;
   if (x >= 0 && y >= 0)   // if both non-negative, then no carry
      c = 0;
   else
   if (x < 0 && y < 0)     // if both negative, then carry
      c = 1;
   else
   if (sum >= 0)           // if signs differ and sum non-neg, then carry
      c = 1;
   else                    // if signs differ and sum neg, then no carry
      c = 0;
   // if signs differ then no overflow
   if ((x < 0 && y >= 0) || (x >= 0 && y < 0))
      v = 0;
   else
   // if signs the same and sum has different sign, then overflow
   if ((sum < 0 && x >= 0) || (sum >= 0 && x < 0))
      v = 1;
   else
      v = 0;
}

int main(int argc, char *argv[])
{
   time_t timer;

   if (argc != 2)
   {
       printf("Wrong number of command line arguments\n");
       printf("Usage: i1 <input filename>\n");
       exit(1);
   }

   // display your name, command line args, time
   time(&timer);      // get time
   printf("Moss Amann     %s %s     %s", 
           argv[0], argv[1], asctime(localtime(&timer)));

   infile = fopen(argv[1], "rb"); // open file in binary mode
   if (!infile)
   {
      printf("Cannot open input file %s\n", argv[1]);
      exit(1);
   }

   fread(&letter, 1, 1, infile);  // test for and discard get file sig
   if (letter != 'o')
   {
      printf("%s not an lcc file", argv[1]);
      exit(1);
   }
   fread(&letter, 1, 1, infile);  // test for and discard 'C'
   if (letter != 'C')
   {
      printf("Missing C header entry in %s\n", argv[1]);
      exit(1);
   }

   fread(mem, 1, sizeof(mem), infile); // read machine code into mem

   while (1)
   {
      // fetch instruction, load it into ir, and increment pc
      ir = mem[pc++];                    

      // isolate the fields of the instruction in the ir
      opcode = ir >> 12;                  // get opcode
      pcoffset9 = ir << 7;                // left justify pcoffset9 field
      pcoffset9 = imm9 = pcoffset9 >> 7;  // sign extend and rt justify
      pcoffset11 = ir << 5;               // left justify pcoffset11 field ****MODIFIED
      pcoffset11 = pcoffset11 >> 5;       // sign extend and rt justify *****MODIFIED
      imm5 = ir << 11;                    // left justify imm5 field ****MODIFIED
      imm5 = imm5 >> 11;                  // sign extend andd rt justify ****MODIFIED
      offset6 = ir << 10;                 // left justify offset6 field ****MODIFIED
      offset6 = offset6 >> 10;            // sign extend and rt justify ****MODIFIED
      eopcode = ir & 0x1f;                // get 5-bit eopcode field    
      trapvec = ir & 0xff;                // get 8-bit trapvec field 
      code = dr = sr = (ir & 0x0e00) >> 9; // get code/dr/sr and rt justify ****MODIFIED
      sr1 = baser = (ir & 0x01c0) >> 6;   // get sr1/baser and rt justify
      sr2 = (ir & 0x0007);                      // get third reg field ***MODIFIED
      bit5 = (ir & 0x0020) >> 5;                // get bit 5. ***MODIFIED
      bit11 = (ir & 0x0800);                // get bit 11.  ***MODIFIED

      // decode (i.e., determine) and execute instruction just fetched
      switch (opcode)
      {
         case 0:                          // branch instructions
            switch(code)  
            {
               case 0: 
                  if (z == 1) {             // brz
                     pc = pc + pcoffset9; 
                     break;
                  } else {
                     pc = pc;
                     break;
                  }
               case 1: 
                  if (z == 0) {            // brnz
                     pc = pc + pcoffset9; 
                     break;
                  } else {
                     pc = pc;
                     break;
                  }
               case 2: 
                  if (n == 1)  {             // brn
                     pc = pc + pcoffset9; 
                     break;
                  } else {
                     pc = pc;
                     break;
                  }
               case 3: 
                  if (n == z)  {          // branch on positive
                     pc = pc + pcoffset9; 
                     break;
                  } else {
                     pc = pc;
                     break;
                  }
               case 4: 
                  if (n != v) {            // branch on less than
                     pc = pc + pcoffset9; 
                     break;
                  } else {
                     pc = pc;
                     break;
                  }
               case 5: 
                  if ((n == v) && (z == 0)) { // branch on greater than
                     pc = pc + pcoffset9; 
                     break;
                  } else {
                     pc = pc;
                     break;
                  }
               case 6: 
                  if (c == 1) {            // branch on carry
                     pc = pc + pcoffset9; 
                     break;
                  } else {
                     pc = pc;
                     break;
                  }
               case 7: 
                  //ir = mem[pc + pcoffset9];    // br
                  pc = pc + pcoffset9;
                  break;
            }                                                   
            break;
         case 1:                               // add
            if (bit5)
            {
               regsave1 = r[sr1];
               r[dr] = regsave1 + imm5;
               // set c, v flags
               setcv(r[dr], regsave1, imm5);
            }   
            else if(~bit5)
            {
               regsave1 = r[sr1]; regsave2 = r[sr2];
               r[dr] = regsave1 + regsave2;
               // set c, v flags
               setcv(r[dr], regsave1, regsave2); 
            }
            // set n, z flags
            setnz(r[dr]);
            break;

         case 2:                          // ld
            r[dr] = mem[pc + pcoffset9];
            break;

         case 3:                          //st
            mem[pc + pcoffset9] = r[sr];
            break;

         case 4:                          //bl & blr
            if(bit11)            
            {
               r[7] = pc; //?
               pc = pc + pcoffset9; 
            } 
            else if(~bit11)
            {
               r[7] = pc; //?
               pc = r[baser] + offset6;
            }
            break;
         case 5:                 // and 
            if (bit5) //sr1 & sr2
            {
               regsave1 = r[sr1]; 
               r[dr] = regsave1 & imm5;
               setcv(r[dr], regsave1, imm5);
            } 
            else if(~bit5)      //sr1 & imm5
            {
               regsave1 = r[sr1];
               regsave2 = r[sr2];
               r[dr] = regsave1 & regsave2;
               setcv(r[dr], regsave1, regsave2);
            }
            setnz(r[dr]);
            break;
         case 6:                //ldr
            r[dr] = mem[r[baser] + offset6];
            break;
         case 7:                //str
            mem[r[baser]+offset6] = r[sr];
            break;
         case 8:                //cmp  //SET FLAGS NO DR
            if(bit5)               //sr1-sr2
            {
               regsave1 = r[sr1];
               setcv(regsave1-imm5, regsave1, imm5);
               setnz(regsave1 - imm5);
                
            }
            else if (~bit5) {
               regsave1 = r[sr1];
               regsave2 = r[sr2];
               setcv(regsave1-regsave2, regsave1, regsave2);
               setnz(regsave1 - regsave2);
            }
            break;
         case 9:                          // not
            // ~ is the not operator in C
            r[dr] = ~r[sr1];
            // set n, z flags
            setnz(r[dr]);
            break;
         case 10:                // push, pop, srl, sra, sll, rol, ror, mul, div, rem, or, xor, mvr, sext
            switch(eopcode) {
                case 0:          //push
                    mem[--r[6]] = sr;
                    break;
                case 1:          //pop
                    dr = mem[r[6]++];
                    break;
                case 2:          //srl, nzc     
                    r[sr] = r[sr] >> ((ir & 0x01e0) >> 5);  
                    setnz(r[sr]);
                    regsave1 = r[sr];
                    if(((regsave1 >> (((ir & 0x01e0) >> 5) - 1)) & 0x8000) == 1 ){
                        c = 1;
                    }
                     //NEED TO SET CARRY FLAG
                    break;
                case 3:          //sra. nzc
                    r[sr] = r[sr] >> ((ir & 0x01e0) >> 5);
                    setnz(r[sr]);
                    //NEED TO SET CARRY FLAG
                    break;
                case 4:          //sll, nzc
                    r[sr] <<= ((ir & 0x01e0) >> 5);
                    setnz(r[sr]);
                    //NEED TO SET CARRY FLAG
                    break;
                case 5:          //rol, nzc
                    regsave2 = sr;
                    while(regsave2 > 0) {
                        regsave1 = r[sr];  //save og num
                        regsave1 = (regsave1 & 0x8000) >> 15; //mask to get MSB & shift it to end
                        r[sr] = (r[sr] << 1) + regsave1; //bit shift and add shifted out bit
                        regsave2 --;
                    }
                    setnz(r[sr]);
                    //setcv(); // determine arguments from function
                    break;
                case 6:          //ror, nzc           
                    regsave2 = sr;  //save amount of rotations, double check value
                    while (regsave2 > 0){
                        regsave1 = r[sr];
                        regsave1 = (regsave1 & 0x0001) << 15;
                        r[sr] = (r[sr] >> 1) + regsave1;
                        regsave2 --;
                    }
                    setnz(r[sr]);
                    //setcv(); // determine arguments from function
                    break;
                case 7:          //mul, nz
                    regsave1 = r[dr];
                    regsave2 = r[sr1];
                    r[dr] = regsave1 * regsave2;
                    setnz(r[dr]);
                    break;
                case 8:          //div, nz
                    regsave1 = r[dr];
                    regsave2 = r[sr1];
                    regsave1 = regsave1 / regsave2;
                    r[dr] = regsave1;
                    setnz(r[dr]);
                    break;
                case 9:          //rem, nz
                    regsave1 = r[dr];
                    regsave2 = r[sr1];
                    regsave1 = regsave1 % regsave2;
                    r[dr] = regsave1;
                    setnz(r[dr]);
                    break;
                case 10:         //or, nz
                    regsave1 = r[dr];
                    regsave2 = r[sr1];
                    r[dr] = regsave1 | regsave2;
                    setnz(r[dr]);
                    break;
                case 11:         //xor, nz
                    regsave1 = r[dr];
                    regsave2 = r[sr1];
                    regsave1 = regsave1 ^ regsave2;
                    r[dr] = regsave1;
                    setnz(r[dr]);
                    break;
                case 12:         //mvr
                    r[dr] = r[sr1];
                    break;
                case 13:         //sext, nz
                    setnz(r[dr]);
                    break;
            }
            break;
         case 11:                // sub
            if(bit5) //sr1 - sr2
            {
                regsave1 = r[sr1];
                r[dr] = regsave1 - imm5;
                setcv(r[dr], regsave1, imm5);
            }
            else if (~bit5) //sr1 - imm5
            {
                regsave1 = r[sr1];
                regsave2 = r[sr2];
                r[dr] = regsave1 - regsave2;
                setcv(r[dr], regsave1, regsave2);
            }
            setnz(r[dr]);
            break;

         case 12:                // jmp/ret
            pc = r[baser] + offset6;                     
            break;

         case 13:                 // mvi
            r[dr] = imm9;
            break;

         case 14:                 // lea
            r[dr] = pc + pcoffset9;
            break;

         case 15:                 // trap
            if (trapvec == 0x00) 
            {            // halt
               exit(0);
            }
            else 
            if (trapvec == 0x01)   //nl
            {          
               printf("\n");
            }
            else 
            if (trapvec == 0x02)  //dout
            {            
               printf("%d", r[sr]);    //dc if sr, sr1, or sr2 //actually, might output binary text. 
                
            }    
            else
            if (trapvec == 0x03)  //udout
            {            
               printf("%d", r[sr]);
            }     
            else
            if (trapvec == 0x04)  //hout
            {            
               printf("%x", r[sr]);
            }  
            else
            if (trapvec == 0x05)  //aout
            {            
               printf("%c", r[sr]);
            }   
            else
            if (trapvec == 0x06)  //sout 
            {            
               regsave1 = r[sr];
               while(mem[regsave1] != '\n'){
                  printf("%c", mem[regsave1]);
                  regsave1 ++;
                  if(mem[regsave1] == '\n') {
                     break;
                  }
               }
               
         
            }    
            else
            if (trapvec == 0x07)  //din
            {            
               scanf("%hd", &r[dr]);
            }     
            else
            if (trapvec == 0x08)  //hin
            {            
               scanf("%hx", &r[dr]);
            }        
            else
            if (trapvec == 0x09)  //ain
            {            
               scanf("%hi", &r[dr]);
            } 
            else
            if (trapvec == 0x10)  //sin
            {            
                while(r[sr] != '\n'){
                    scanf("%hd", &r[sr]);
                    //prob need a sr ++ or something
                }
                
            }                     
            break;
      }     // end of switch
   }        // end of while
}
