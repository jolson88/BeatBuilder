using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using BeatBuilder.Audio;
using System.Threading.Tasks;
using Windows.Media;

namespace BeatBuilder.WinApp
{
    public sealed partial class MainPage : Page
    {
        SystemMediaTransportControls systemControls;
        AudioRenderer renderer;
        DrumPad drumPad;
        Looper looper;
        Oscillator oscillator;

        public MainPage()
        {
            this.InitializeComponent();
            this.Loaded += MainPage_Loaded;
        }

        async void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            this.systemControls = SystemMediaTransportControls.GetForCurrentView();
            this.systemControls.ButtonPressed += systemControls_ButtonPressed;
            this.systemControls.IsPlayEnabled = true;
            this.systemControls.IsPauseEnabled = true;

            this.renderer = await AudioRenderer.CreateAsync();

            var rootPath = Windows.ApplicationModel.Package.Current.InstalledLocation.Path + "\\Sounds\\";
            this.drumPad = new DrumPad();
            this.drumPad.SetDrumSound(DrumKind.Bass, rootPath + "Drum-Bass.wav");
            this.drumPad.SetDrumSound(DrumKind.Snare, rootPath + "Drum-Snare.wav");
            this.drumPad.SetDrumSound(DrumKind.Shaker, rootPath + "Drum-Shaker.wav");
            this.drumPad.SetDrumSound(DrumKind.ClosedHiHat, rootPath + "Drum-Closed-Hi-Hat.wav");
            this.drumPad.SetDrumSound(DrumKind.Cowbell, rootPath + "Cowbell.wav");
            this.drumPad.SetDrumSound(DrumKind.OpenHiHat, rootPath + "Drum-Open-Hi-Hat.wav");
            this.drumPad.SetDrumSound(DrumKind.RideCymbal, rootPath + "Drum-Ride-Cymbal.wav");
            this.drumPad.SetDrumSound(DrumKind.FloorTom, rootPath + "developer_loud.wav");
            this.drumPad.SetDrumSound(DrumKind.HighTom, rootPath + "satya_fantastic.wav");

            this.oscillator = new Oscillator();

            this.looper = new Looper();
            
            //this.renderer.ListenTo(this.looper);
            this.renderer.ListenTo(this.oscillator);

            this.looper.ListenTo(this.drumPad);

            Play();
        }

        void systemControls_ButtonPressed(SystemMediaTransportControls sender, SystemMediaTransportControlsButtonPressedEventArgs args)
        {
            switch (args.Button)
            {
                case SystemMediaTransportControlsButton.Play:
                    Play();
                    break;

                case SystemMediaTransportControlsButton.Pause:
                    Pause();
                    break;

                default:
                    break;
            }
        }

        private void Play()
        {
            this.renderer.Start();
            this.systemControls.PlaybackStatus = MediaPlaybackStatus.Playing;
        }

        private void Pause()
        {
            this.renderer.Stop();
            this.systemControls.PlaybackStatus = MediaPlaybackStatus.Paused;
        }

        private void BassButton_Click(object sender, RoutedEventArgs e)
        {
            this.drumPad.PlayDrum(DrumKind.Bass);
        }

        private void SnareButton_Click(object sender, RoutedEventArgs e)
        {
            this.drumPad.PlayDrum(DrumKind.Snare);
        }

        private void ShakerButton_Click(object sender, RoutedEventArgs e)
        {
            this.drumPad.PlayDrum(DrumKind.Shaker);
        }

        private void ClosedHiHatButton_Click(object sender, RoutedEventArgs e)
        {
            this.drumPad.PlayDrum(DrumKind.ClosedHiHat);
        }

        private void CowbellButton_Click(object sender, RoutedEventArgs e)
        {
            this.drumPad.PlayDrum(DrumKind.Cowbell);
        }

        private void OpenHiHatButton_Click(object sender, RoutedEventArgs e)
        {
            this.drumPad.PlayDrum(DrumKind.OpenHiHat);
        }

        private void RideCymbalButton_Click(object sender, RoutedEventArgs e)
        {
            this.drumPad.PlayDrum(DrumKind.RideCymbal);
        }

        private void FloorTomButton_Click(object sender, RoutedEventArgs e)
        {
            this.drumPad.PlayDrum(DrumKind.FloorTom);
        }

        private void HighTomButton_Click(object sender, RoutedEventArgs e)
        {
            this.drumPad.PlayDrum(DrumKind.HighTom);
        }

        private void RecordStartButton_Click(object sender, RoutedEventArgs e)
        {
            this.looper.StartRecording();
        }

        private void RecordStopButton_Click(object sender, RoutedEventArgs e)
        {
            this.looper.StopRecording();
        }

        private void ResetButton_Click(object sender, RoutedEventArgs e)
        {
            this.looper.ResetLoops();
        }

        private void Bass_PointerPressed(object sender, PointerRoutedEventArgs e)
        {
            this.drumPad.PlayDrum(DrumKind.Bass);
        }

        private void OscillatorTestButton_PointerPressed(object sender, PointerRoutedEventArgs e)
        {

        }

        private void OscillatorTestButton_PointerReleased(object sender, PointerRoutedEventArgs e)
        {

        }
    }
}
