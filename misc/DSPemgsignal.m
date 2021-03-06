close all
clear all
clc

%s = serialport('/dev/cu.usbmodem14203', 230400, "Timeout",1);

% DUMMY
y = importdata('data.txt');
y = y(1:5500);

K = 300;
tmp = zeros(1, numel(y));
vect = [1:K:numel(y)];
vect(end+1) = numel(y);

for i = 1:numel(vect)-1
    tmp(vect(i):vect(i+1)) = DSP(y(vect(i):vect(i+1)));
end

data = [];
data2 = [];
subplot(2,1,1);
subplot(2,1,2);
ylim([-2 2])

i = 0;
while(1)
    %data(end+1:end+300) = read(s, 300, 'single'); 
    
    data(end+1:end+K) = tmp(i*K+1:(i+1)*K); % DUMMY
    data2(end+1:end+K) = y(i*K+1:(i+1)*K); % DUMMY
    
    subplot(2,1,1), plot(data2, 'LineWidth', 1),
    legend('Raw EMG');
    
    subplot(2,1,2), plot(data, 'k', 'LineWidth', 1),
    legend('Filtered EMG');
    
    i = i+1;
    
    pause(0.3);
end


function processedVector = DSP(dataVector)

processedVector = zeros(1, numel(dataVector));

%G_bandpass = [0.613269623000704;0.613269623000704;0.531457333626825;1];
%SOS_bandpass = [1,0,-1,1,-1.80705171170818,0.841553594017866;1,0,-1,1,0.396533783709051,0.400268204833385;1,0,-1,1,-0.721316627559154,-0.0629146672536498];
b_bandpass = [0.199880906801133,0,-0.599642720403399,0,0.599642720403399,0,-0.199880906801133];
a_bandpass = [1,-2.13183455555828,1.47978011393210,-0.679740843101842,0.584825906895303,-0.218461835750097,-0.0211926261278646];
%[b_bandpass, a_bandpass] = sos2tf(SOS_bandpass, G_bandpass);

%G_stopband = [0.995566971680792;0.995566971680792;1];
%SOS_stopband = [1,-1.90215057941424,1,1,-1.89085146346236,0.991033147905607;1,-1.90215057941424,1,1,-1.89654806962015,0.991274058481536];
b_stopband = [0.991153595101663,-3.77064677042227,5.56847615976590,-3.77064677042227,0.991153595101663];
a_stopband = [1,-3.78739953308251,5.56839789935512,-3.75389400776205,0.982385450614124];
%[b_stopband, a_stopband] = sos2tf(SOS_stopband, G_stopband);

filtered = filtfilt(b_bandpass, a_bandpass, dataVector);
filtered = filtfilt(b_stopband, a_stopband, filtered);

processedVector = filtered;

end

