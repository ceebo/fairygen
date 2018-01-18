EGTgen 1.0 release notes

EGTgen is simple in-memory tablebase builder, which after the building
allows probing of the tablebase, so you can try to defend the end-game
against it. It was especially designed for determining mating potential
of unorthodox pieces, and assumes 8x8 board and octo-symmetric pieces.
(So it cannot be used with Pawns). A compiler switch DIAGSYM allows you
to compile a version that only assumes 4-fold-symmetry, at the expense
of increasing memory use almost two-fold (e.g. 256MB in stead of 160MB
for a 5-men). The typical requirements are

5-men 160MB 10-20 min
4-men   3MB 5-10 sec
3-men  40KB < 1 sec

Each number of men needs a different version, compiled separately.

H.G.Muller
Nov 1, 2011


                         EGTgen USER MANUAL

Syntax

     Nmen [-xboard] [-m] [-0] [-s] [-f SEEDFILE] ENDGAMESPEC

Basic functionality

  To tell what end-game EGTgen has to build a tablebase for, you have to
  use the proper version (for the number of men you want), and specify
  the end-game on its command line. E.g.

     4men KQ.KR
     5men KQN.KA

  Note that the first piece for each color is taken to be the royal one;
  if you specify NKQ.KQ it is not the same at all as KQN.KQ, because now
  black will try to mate or give a perpetual on the Knight, and the white
  King would be a sacrificeable piece. The royal pieces of both sides do
  not have to be of the same type.

  After the EGT is generated, you can probe it. The EGT is not saved in
  any way, and disappears when you quit the program.

Options

  -xboard   Lets EGT communicate in XBoard protocol, so you can run
            it as an engine under WinBoard.

  -m        Use DTM as metric rather than DTC.

  -0        (null!) Generate the EGT under rules where black is allowed to
            pass his turn when not stalemated. Useful for investigating
            how important zugzwang is in the end-game.

  -s        Generate the EGT under rules where stalemate is a win,
            rather than a draw.

  -f FILE   The specified file is used to set the DTx of the positions
            mentioned in it to the given DTx. For the format of this
            file, see below.

Detailed description of operation

  The tablebase is built for white wins vs non-wins. Normally it uses DTC
  (distance to conversion or mate) as metric, but under control of a -m
  flag on the command line it can build DTM as well. To know if a non-win 
  is a draw or loss, you have to build a tablebase with swapped material. 
  Tablebase statistics are always written on a file rep2.txt (overwriting 
  it when it existed). EGTgen always builds all sub-set end-games together
  with the specified end-game, but if you are building DTC the sub-set
  end-games show all up as DTC=1 in the statistics, and the lost-in-n
  positions start counting at 10. (So 10 = checkmated with all pieces.)

  After generation, you can probe the tablebase for white moves from
  a given position. It then produces a list of pseudo-legal moves from
  that position, plus the DTx of the positions they lead to. It also prints
  the best move (i.e. to a position with lowest DTx). You can enter a
  position by typing the index, or (recommended) specifying it in WB protocol
  through the good old 'edit' command. You can also type a move, which
  then is used to alter the position to which the previously reported
  best move led, so you can actually play out a mating sequence, where
  the generator beats you with optimal play. As the move and positions
  can be entered / printed in WB protocol format, the easiest way to
  do this is to install EGTgen as an engine under WinBoard / XBoard.

  Apart from the letters for orthodox pieces (all upper case!) EGTgen pre-
  defines a large number of fairy pieces, such as Archbishop A, Ferz x,
  Alfil e, etc. To know which letter indicates what, you have to take
  a peek at the source (the leading comment and array cc[]). As WinBoard
  is not likely to know the pieces by the same letter (if it knows them
  at all...), it is often better to forget about the built-in types,
  and define your own pieces. You can do that by putting a file
  "piecedef.ini" in the folder where EGTgen runs. The distribution comes
  with such a file, which defines pieces the way WinBoard uses them
  For the format of this file, see below.

Using EGTgen as WinBoard engine

  To use EGTgen as WB engine, you have to specify an extra (first)
  argument '-xboard' on its command line. The best way to install
  it under WinBoard is to add the line

  "4men -xboard KQ.KR" /fd="../EGTgen" /initialMode=EditPosition /variant=fairy

  to the /firstChessProgramNames in the WinBoard settings file (using
  the "Edit engine list" item in the Engine menu), so that you can 
  immediately start to enter a position from the end-game you
  requested (KQKR in the example), and will see the building stats
  scroll through the Engine-Output window. After the building is
  finished, and you set up a position, you can then click 'Machine White'
  to play against it from that position. EGTgen can only play white.
  Note that EGTgen has an extremely flaky implementation of WB protocol;
  even if you would tell it to play black, it would still move the white
  pieces! 'Retract move' should work, though.

  The current version of EGTgen will only be able to play upto the
  conversion in DTC; as soon as something is captured, you will leave
  the generated tablebase, and it would not know what to do anymore.
  You might have to play with legality testing switched off in WinBoard;
  EGTgen will complain in most cases if you enter a move that is
  not pseudo-legal (and captures your King in some other cases! :-))) ).

Seeding the EGT

  Since version 1.2 it is possible to 'seed' the EGT, by arbitrarily
  defining the value of the DTx metric for selectedpositions. This can
  be done by specifying a file on the command-line with the extra
  option "-f FILENAME". The file contains lines that specify a position 
  and a DTx value. This DTx value is then assigned to the mentioned
  position immediately after determining the checkmates (so that you
  can redefine a specific checkmate as draw or undecided).

File formats

                   piecedef.ini file format

  Each line in the piecedef.ini file is taken as a piece definition. 
  The lines start with a piece-ID letter, followed by a colon, and
  a space-separated list of moves, like:

     N: 1,2 1,-2 2,1 2,-1 -2,1 -2,-1 -1,2 -1,-2
     N: 1,2,*
     B: 1,1,s*
     R: 1,0,s*
     Q: 1,0,s* 1,1,s*
     K: 1,0,* 1,1,*
     A: 1,2,* 1,1,s*
     X: 1,1,c* 1,0,n*
     L: 1,1,* 2,2,* 0,1,n+

  The two comma-separated numbers are the coordinates of the move (in
  a special version that only supports 4-fold symmetry, the first number
  would be forward/backward, the second left-right). You can either list
  all possibilities (separated by spaces), or use the symbols * or +
  behind an extra comma to indicate you mean all symmetry-equivalent 
  versions of the given step. (* and + are only different in a 4-fold 
  symmetric version: * would generalize the move to all 8 directions
  even there, and + only to 4.) As you can see in the two possible
  definitions of the Knight, using the * is much easier, and much less
  error prone. (What happens when you enter a set of moves that is
  less symmetric that the generator assumes, the is undefined,
  and unlikely what you intended...) Behind the second comma you can
  also specify other aspects of the move, like 's' if it is a slider
  move in that direction (default would be leaper), 'c' if it can
  only capture, or 'n' if it can only non-capture. (Default is both.)
  So the X defines an 'omni-directional pawn', capturing in the 4
  diagonal directions, and non-capturing in the orthogonal directions.
  The L defines the Lieutenant of Spartan Chess (and would only work
  in a 4-fold-symmetric compile ofEGTgen). The order of the symbols
  specified behind the coordinates is irrelevant.

                         Seeding file format

  The seeding file contains a list of positions plus DTx these should
  get, one per line, like:

      b3e3d2a1 127
      b3e3d2b1 10
      b3b2**b1 10

  The position is given by concatenating the  squares for the pieces,
  first white, than black, but the black pieces are mentioned reversed.
  So b3e3c2b1 in KQ.KR would be w: Kb3,Qe3 b: Kb1,Rc2. The position
  can contain a single wildcard, indicated by ** for the square,
  meaning that the corresponding piece can be anywhere. The DTC code
  starts counting at 10 for checkmated, then 11 for mated-in-1, etc.
  in DTC mode for the end-game with all pieces present. In DTM mode
  the counting is DTM=1 for checkmated, 2 for mate-in-1, etc.
  Special DTX codes are 0 (undecided), 127 (stalemate) and 126 (won 
  for black).

  A given DTx will always be applied to all symmetry-equivalent 
  positions.


                          CHANGE LOG

Version 1.1, Nov 10, 2011
* fixed bug in probing (wrong diag symmetry)
* fixed wrong determination of color-binding from piecedef.ini
* added -0 option, which allows black to null-move
* added -s option, which makes stalemate a win (-0 -s = forced pass)
* ask user if he wants to continue after DTC overflows
* use tabs in xboard output for less jumpy formatting
* print XBoard thinking output for probe (DTC score)
* make XBoard also show (octal) maximin positions
* command syntax now is "4men [-xboard] [-0] [-s] KBB.K"

Version 1.2, Nov 17, 2011
* added posibility to 'seed' arbitrary positions to arbitrary state
* changed treatment of stalemate when null-moving allowed
* added flag for building DTM in stead of DTC
* improved description README file
* new command syntax: "4men [-xboard] [-m] [-0] [-s] [-f FILENAME] KBB.K"
