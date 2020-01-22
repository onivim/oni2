 
(* camlp5r *)
(* diff.ml,v *)
(* Copyright (c) INRIA 2007-2017 *)

(* Parts of Code of GNU diff (analyze.c) translated from C to OCaml
   and adjusted. Basic algorithm described by Eugene W.Myers in:
     "An O(ND) Difference Algorithm and Its Variations" *)

     exception DiagReturn of int;;
     
     let diag fd bd sh xv yv xoff xlim yoff ylim =
       let dmin = xoff - ylim in
       let dmax = xlim - yoff in
       let fmid = xoff - yoff in
       let bmid = xlim - ylim in
       let odd = (fmid - bmid) land 1 <> 0 in
       fd.(sh+fmid) <- xoff;
       bd.(sh+bmid) <- xlim;
       try
         let rec loop fmin fmax bmin bmax =
           let fmin =
             if fmin > dmin then begin fd.(sh+fmin-2) <- -1; fmin - 1 end
             else fmin + 1
           in
           let fmax =
             if fmax < dmax then begin fd.(sh+fmax+2) <- -1; fmax + 1 end
             else fmax - 1
           in
           begin let rec loop d =
             if d < fmin then ()
             else
               let tlo = fd.(sh+d-1) in
               let thi = fd.(sh+d+1) in
               let x = if tlo >= thi then tlo + 1 else thi in
               let x =
                 let rec loop xv yv xlim ylim x y =
                   if x < xlim && y < ylim && xv x == yv y then
                     loop xv yv xlim ylim (x + 1) (y + 1)
                   else x
                 in
                 loop xv yv xlim ylim x (x - d)
               in
               fd.(sh+d) <- x;
               if odd && bmin <= d && d <= bmax && bd.(sh+d) <= fd.(sh+d) then
                 raise (DiagReturn d)
               else loop (d - 2)
           in
             loop fmax
           end;
           let bmin =
             if bmin > dmin then begin bd.(sh+bmin-2) <- max_int; bmin - 1 end
             else bmin + 1
           in
           let bmax =
             if bmax < dmax then begin bd.(sh+bmax+2) <- max_int; bmax + 1 end
             else bmax - 1
           in
           begin let rec loop d =
             if d < bmin then ()
             else
               let tlo = bd.(sh+d-1) in
               let thi = bd.(sh+d+1) in
               let x = if tlo < thi then tlo else thi - 1 in
               let x =
                 let rec loop xv yv xoff yoff x y =
                   if x > xoff && y > yoff && xv (x - 1) == yv (y - 1) then
                     loop xv yv xoff yoff (x - 1) (y - 1)
                   else x
                 in
                 loop xv yv xoff yoff x (x - d)
               in
               bd.(sh+d) <- x;
               if not odd && fmin <= d && d <= fmax && bd.(sh+d) <= fd.(sh+d) then
                 raise (DiagReturn d)
               else loop (d - 2)
           in
             loop bmax
           end;
           loop fmin fmax bmin bmax
         in
         loop fmid fmid bmid bmid
       with DiagReturn i -> i
     ;;
     
     let diff_loop a ai b bi n m =
       let fd = Array.make (n + m + 3) 0 in
       let bd = Array.make (n + m + 3) 0 in
       let sh = m + 1 in
       let xvec i = a.(ai.(i)) in
       let yvec j = b.(bi.(j)) in
       let chng1 = Array.make (Array.length a) true in
       let chng2 = Array.make (Array.length b) true in
       for i = 0 to n - 1 do chng1.(ai.(i)) <- false done;
       for j = 0 to m - 1 do chng2.(bi.(j)) <- false done;
       let rec loop xoff xlim yoff ylim =
         let (xoff, yoff) =
           let rec loop xoff yoff =
             if xoff < xlim && yoff < ylim && xvec xoff == yvec yoff then
               loop (xoff + 1) (yoff + 1)
             else xoff, yoff
           in
           loop xoff yoff
         in
         let (xlim, ylim) =
           let rec loop xlim ylim =
             if xlim > xoff && ylim > yoff && xvec (xlim - 1) == yvec (ylim - 1)
             then
               loop (xlim - 1) (ylim - 1)
             else xlim, ylim
           in
           loop xlim ylim
         in
         if xoff = xlim then
           for y = yoff to ylim - 1 do chng2.(bi.(y)) <- true done
         else if yoff = ylim then
           for x = xoff to xlim - 1 do chng1.(ai.(x)) <- true done
         else
           let d = diag fd bd sh xvec yvec xoff xlim yoff ylim in
           let b = bd.(sh+d) in loop xoff b yoff (b - d); loop b xlim (b - d) ylim
       in
       loop 0 n 0 m; chng1, chng2
     ;;
     
     (* [make_indexer a b] returns an array of index of items of [a] which
        are also present in [b]; this way, the main algorithm can skip items
        which, anyway, are different. This improves the speed much.
          The same time, this function updates the items of so that all
        equal items point to the same unique item. All items comparisons in
        the main algorithm can therefore be done with [==] instead of [=],
        what can improve speed much. *)
     let make_indexer a b =
       let n = Array.length a in
       let htb = Hashtbl.create (10 * Array.length b) in
       Array.iteri
         (fun i e ->
            try b.(i) <- Hashtbl.find htb e with Not_found -> Hashtbl.add htb e e)
         b;
       let ai = Array.make n 0 in
       let k =
         let rec loop i k =
           if i = n then k
           else
             let k =
               try
                 a.(i) <- Hashtbl.find htb a.(i);
                 (* line found (since "Not_found" not raised) *)
                 ai.(k) <- i;
                 k + 1
               with Not_found -> k
             in
             loop (i + 1) k
         in
         loop 0 0
       in
       Array.sub ai 0 k
     ;;
     
     let f a b =
       let ai = make_indexer a b in
       let bi = make_indexer b a in
       let n = Array.length ai in
       let m = Array.length bi in diff_loop a ai b bi n m
     ;;