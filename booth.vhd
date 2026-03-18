library ieee;
use ieee.std_logic_1164.all;

-- This part (ENTITY) declares inputs to and outputs from the system - Booth Multiplier
--The circuit multiplies two signed 6-bit numbers and gives a 12-bit signed result

entity Booth is
  generic (NofB : integer := 6);                                  -- Number of bits in each input
  port (MULD    : in std_logic_vector((NofB-1) downto 0);    -- n-bit multiplicand
        MULR    : in std_logic_vector((NofB-1) downto 0);    -- n-bit multiplier
	RST     : in std_logic;                              -- Active-LOW reset input for initialization
	CLK     : in std_logic;                              -- System clock input
	ClkB    : in std_logic;                              -- System clock input 
	MULT    : in std_logic;                              -- Active-HIGH input to initiate multiplication
	RSLT    : out std_logic_vector((2*NofB-1) downto 0)); -- 2*n-bit multiplication result
end Booth;

architecture RTL of Booth is


--A +B when Sub=0
--A -B when Sub=1
  component AddSub
    generic (n : integer);
    port (A    : in std_logic_vector((n-1) downto 0);   -- n-bit number for addition or subtraction
          B    : in std_logic_vector((n-1) downto 0);   -- n-bit number for addition or subtraction
          Sub  : in std_logic;                          -- S = '0' for addition, '1' for subtraction
	      COUT : out std_logic;                         -- Carry flag
          SUM  : out std_logic_vector((n-1) downto 0)); -- n-bit result
  end component;

-- shift register operates at the positive edges in its clock input (CLK)
-- if LOAD =1, it loads DATA if not it shits right by 1 bit
--SIN goes into the left side, MSB side

  component SHIFT
    generic (n   : integer);                               -- n-bit shifter register.
    port (CLK    : in std_logic;                           -- At positive edges of its clock, the shifter either load
                                                           -- the data input or shift its content a bit to the right.
                                                           -- The msb of the shift register is filled with the serial input (SIN).                       
	  ENABLE : in std_logic;                           -- This active-HIGH input is to enable the shift register.
	  LOAD   : in std_logic;                           -- This active-HIGH input is to load a parallel data input, DATA. 
	  SIN    : in std_logic;                           -- Serial input, which shifts into the MSB 
      DATA   : in std_logic_vector((n-1) downto 0);    -- Data to be loaded into the shift register when LOAD input is active.     
      SHFT_O : out std_logic_vector((n-1) downto 0));  -- Content stored in the shift register
  end component;

--The counter counts how many Booth steps happened
--When there is enough shifts done, multiplication is finished
  component CNT
    generic (n  : integer);
    port (CLK   : in std_logic;                           -- Positive edges are active clock edges
          RST   : in std_logic;                           -- Active-LOW reset input for initialization
          En    : in std_logic;
          CNT_O : out std_logic_vector((n-1) downto 0));  -- n-bit counter output
  end component;

--SM controls the multiplier
--It goes through reset, load and shift states

  component SM
    port (CLK      : in std_logic;                       -- Positive edges are active clock edges
          RSTB     : in std_logic;                       -- Active-LOW; RST_ST output is asserted
          MULT     : in std_logic;                       -- Acitve-HIGH; LOAD_ST is asserted
          DONE     : in std_logic;                       -- Active-HIGH; RST_ST outpur is asserted
          RST_ST   : out std_logic;                      -- Reset state: the multiplier stays idle
          LOAD_ST  : out std_logic;                      -- Load state: two numbers are loaded,
          SHIFT_ST : out std_logic);                     -- Shift state: add/shift steps are repeated
  end component;

-- Internal signals to make interconnections among components

  signal RstB               : std_logic;                 -- inverted power-on reset
  signal D0                 : std_logic;                 -- three states from the state machine, value used to update Q0
  signal ShiftEn            : std_logic;                 -- enables shift registers
  signal Ovf, Cf, Nf, Zf    : std_logic;                 -- condition flags
  signal LdSum, Load, Q0    : std_logic;                 -- control signals for loading & Booth logic
  signal Done, SinL         : std_logic;                 -- serial input for lower shifter and done signal
  signal RstSt, LdSt, ShtSt : std_logic;                 -- three states from the state machine
  signal Sub                : std_logic;                 -- control signal for ALU tells it to add or sub
  signal Cnt_O     : std_logic_vector(2 downto 0);       -- counter output
  signal BoothCode : std_logic_vector(1 downto 0);       -- determines whether to add/sub, curr booth code
  signal A         : std_logic_vector((NofB-1) downto 0);   -- upper half input
  signal M         : std_logic_vector((NofB-1) downto 0);   -- register to hole the multiplicand
  signal Sum       : std_logic_vector((NofB-1) downto 0);   -- output from ALU
  signal Shft_O    : std_logic_vector((2*NofB-1) downto 0); -- full result register
 
 begin

  RstB   <= not RST; --internal reset signal

--Storing the multiplicand during the load state
  Multiplicand : process(ClkB, LdSt, MULD)  
  begin                                      
    if ClkB'event and ClkB = '1' then
      if LdSt = '1' then
        M <= MULD;
      end if;
    end if;
  end process;

--previous bit starts at 0 during the load state
-- if not it uses the current LSB of the shifter
  D0 <= '0' when LdSt = '1' else Shft_O(0);
  --Shift registers working in load state or shift state
  ShiftEn <= (LdSt or ShtSt); 

--Saving the previous LSB bit for Booth coding
  process(ClkB, ShiftEn, D0)     
  begin                                     
    if ClkB'event and ClkB = '1' then
      if ShiftEn = '1' then                   
        Q0 <= D0;                           
      end if;                                 
    end if;
  end process;

  BoothCode <= Shft_O(0) & Q0;  --booth code using current LSB and previous LSB

--if booth code is 01 or 10 we add/sub
  LdSum <= BoothCode(1) XOR BoothCode(0);  
--loading happens when we are in load stae or after add/sub is ready                                        
  Load  <= LdSt or LdSum;                   
                                         

--A being the input for the upper shifter
--in the load state it becomes 0 otherwise it gets the shifted ALU result
  A <= (others => '0') when LdSt = '1' else Sum(NofB-1) & Sum((NofB-1) downto 1); 
  
  SinL <= Sum(0) when LdSum = '1' else Shft_O(NofB);                           


--upper shift register holding the upper half of the product
  ShifterH : SHIFT                                  
    generic map (n   => NofB)                     
    port map (CLK    => ClkB,                                                      
              ENABLE => ShiftEn,
              LOAD   => Load,                      
              SIN    => Shft_O((2*NofB-1)),     
              DATA   => A,               
              SHFT_O => Shft_O((2*NofB-1) downto NofB));  

--lower shift register starts with multiplier and later holds the lower result bits
  ShifterL : SHIFT                                  
    generic map (n   => NofB)     
    port map (CLK    => ClkB,
              ENABLE => ShiftEn,
              LOAD   => LdSt,                       
              SIN    => SinL,                       
	          DATA   => MULR,                       
              SHFT_O => Shft_O((NofB-1) downto 0));

--final output
--combined shift register
  RSLT <= Shft_O;                                 

--boothcode to decide to add or subtract
  Sub <= BoothCode(1); 

--upper half of the result and multiplicand worked on by ALU
  AdderSubtractor : AddSub
    generic map (n => NofB)                       
    port map (A    => Shft_O((2*NofB-1) downto NofB),    
              B    => M,
              Sub  => Sub,
	      COUT => Cf,
	      SUM  => Sum );

--runs during shift state
--resets during load state
  Counter : CNT
    generic map (n  => 3)               
    port map (CLK   => CLKB,
              RST   => LdSt,                      
	          EN => ShtSt,
              CNT_O => Cnt_O);

  Done   <= '1' when Cnt_O = "110" else '0';      -- Done becomes HIGH when all add/shift
  
-- SM controls rest,load and shift stages                                                 
  StMachine : SM                                  
    port map (CLK      => CLK,                    
              RSTB     => RstB,
              MULT     => MULT,
              DONE     => Done,
              RST_ST   => RstSt,
              LOAD_ST  => LdSt,
              SHIFT_ST => ShtSt);

end RTL;
