/*
   Author : D. Menasce
   Usage  : STDLINE(string) ;
   Purpose: Formatter of console.log printouts in the Firefox javascript debugger
*/

function STDLINE(str) 
{
  const e = new Error();
  const a = e.stack.split("\n")[1] ;
  const w = a.split("/") ;
  const s = w.length -1 ;
  const l = w[s].split(":")[1] ;
  const n = w[s].split(":")[0] ;
  const m = l+"] ["+n+"] "+str ;
  console.log(m) ;
}

function STDGETLNUM()
{
  const e = new Error();
  const a = e.stack.split("\n")[1] ;
  const w = a.split("/") ;
  const s = w.length -1 ;
  const l = w[s].split(":")[1] ;
  const n = w[s].split(":")[0] ;
  return l ;
}

