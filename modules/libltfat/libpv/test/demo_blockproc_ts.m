function demo_blockproc_ts(source,varargin) %RUNASSCRIPT
%DEMO_BLOCKPROC_BASICLOOP Basic real-time audio manipulation
%   Usage: demo_blockproc_basicloop('gspi.wav')
%
%   For additional help call |demo_blockproc_basicloop| without arguments.
%
%   The demo runs simple playback loop allowing to set gain in dB.
% 

if demo_blockproc_header(mfilename,nargin)
   return;
end

initstretch = log(2109/1024)/log(4);
maxstretch = 4;
% Basic Control pannel (Java object)
p = blockpanel({
               {'GdB','Gain',-20,20,0,21},...
               {'stretch','Stretch',-1,1,initstretch,501},
               });
         
% Setup blocktream
try
    fs = block(source,varargin{:},'loadind',p,'single');
catch
    % Close the windows if initialization fails
    blockdone(p);
    err = lasterror;
    error(err.message);
end

% Set buffer length to 30 ms
Lout = floor(30e-3*fs);
Loutvar = 100;

plan = libpointer();
status = calllib('libpv','ltfat_pv_init_s', maxstretch, 1, Lout + Loutvar ,plan);

fout = zeros(Lout + Loutvar,1,'single');

flag = 1;
%Loop until end of the stream (flag) and until panel is opened
while flag && p.flag
   gain = blockpanelget(p,'GdB');
   stretch = (blockpanelget(p,'stretch'));
   if (stretch >= 0)
       stretch = 1 + 3*stretch;
   else
       stretch = 1 + stretch*3/4;
   end
   
   Louttmp = randi([-Loutvar,Loutvar]) + Lout;
 
   %calllib('libpv','print_pos', plan);
   Lin = calllib('libpv','ltfat_pv_nextinlen_s',plan,Louttmp);
   %fprintf('stretch=%.3f, Lout=%i, Lin=%i\n',stretch,Lout,Lin);
       
   gain = 10^(gain/20);
   
   [f,flag] = blockread(Lin);
   
   bufInPtr = libpointer('singlePtr',f(:,1));
   bufOutPtr = libpointer('singlePtr',fout);
   calllib('libpv','ltfat_pv_execute_s',plan,bufInPtr,Lin,1,stretch,Louttmp,bufOutPtr);
   fout2 = bufOutPtr.Value;
   foutres = fout2(1:Louttmp)*gain;
   %foutres = dctresample(foutres,Lintmp);
   
   % The following does nothing in the rec only mode.
   blockplay(foutres);
   % The following does nothing if 'outfile' was not specified 
   blockwrite(foutres);
end
blockdone(p);
calllib('libpv','ltfat_pv_done_s',plan);