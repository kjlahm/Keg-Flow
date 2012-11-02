using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace KegCommunicator {
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window {

        public List<string> ports;
        private int LEFT_sensorPulses;
        private int RIGHT_sensorPulses;
        private string LEFT_beerName;
        private string RIGHT_beerName;

        public MainWindow() {
            InitializeComponent();

            ports = new List<string>();
        }

        private void BC_send_Click(object sender, RoutedEventArgs e) {
            try {
                int red = Convert.ToInt32(BC_red.Text);
                int green = Convert.ToInt32(BC_green.Text);
                int blue = Convert.ToInt32(BC_blue.Text);

                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                BC_feedback.Text = sc.send_command("BC " + red + " " + green + " " + blue);
            }
            catch (InvalidCastException ex) {
                BC_feedback.Text = ex.Message;
            }
            catch (Exception ex) {
                BC_feedback.Text = ex.Message;
            }
        }

        private void SP_send_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                string command = "SP ";

                if (Convert.ToBoolean(SP_left.IsChecked))
                {
                    command += "L ";
                    if (Convert.ToBoolean(SP_useSaved.IsChecked))
                    {
                        command += format_pulse(LEFT_sensorPulses);
                    }
                    else
                    {
                        command += format_pulse(Convert.ToInt32(SP_newPulses.Text));
                    }
                }
                else
                {
                    command += "R ";
                    if (Convert.ToBoolean(SP_useSaved.IsChecked))
                    {
                        command += format_pulse(RIGHT_sensorPulses);
                    }
                    else
                    {
                        command += format_pulse(Convert.ToInt32(SP_newPulses.Text));
                    }
                }

            }
            catch (Exception ex)
            {
                SP_feedback.Text = ex.Message;
            }
        }

        /// <summary>
        /// Formats the given pulse value to a 4 character string. For values greater than 9999 the 
        /// leading digits will be cut off.
        /// </summary>
        /// <param name="pulse">Pulse value to be formatted.</param>
        /// <returns>4 digit string of the given pulse.</returns>
        string format_pulse(int pulse)
        {
            // Trim of leading digits (we can only send 4)
            while (pulse > 9999)
            {
                pulse -= 10000;
            }

            // Add leading zeros if the response isn't 4 digits.
            string response = Convert.ToString(pulse);
            while (response.Length < 4)
            {
                response = " " + response;
            }

            return response;
        }

        private void GP_send_Click(object sender, RoutedEventArgs e) {
            try {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                string response;
                if (Convert.ToBoolean(GP_left.IsChecked)) {
                    response = sc.send_command("GP L");
                    LEFT_sensorPulses = Convert.ToInt32(response);
                }
                else {
                    response = sc.send_command("GP R");
                    RIGHT_sensorPulses = Convert.ToInt32(response);
                }
                GP_feedback.Text = response;
            }
            catch (Exception ex) {
                GP_feedback.Text = ex.Message;
            }
        }

        private void SB_send_Click(object sender, RoutedEventArgs e) {
            try {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                string command = "SB ";
                if (Convert.ToBoolean(SB_left.IsChecked)) {
                    command += "L ";
                    if (Convert.ToBoolean(SB_useSaved.IsChecked))
                    {
                        command += LEFT_beerName;
                    }
                    else
                    {
                        command += SB_newBeer.Text;
                    }
                }
                else {
                    command += "R ";
                    if (Convert.ToBoolean(SB_useSaved.IsChecked))
                    {
                        command += RIGHT_beerName;
                    }
                    else
                    {
                        command += SB_newBeer.Text;
                    }
                }
                SB_feedback.Text = sc.send_command(command);
            }
            catch (Exception ex) {
                SB_feedback.Text = ex.Message;
            }
        }

        private void GB_send_Click(object sender, RoutedEventArgs e) {
            try {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                string response;
                if (Convert.ToBoolean(GB_left.IsChecked)) {
                    response = sc.send_command("GB L");
                    LEFT_beerName = response;
                }
                else {
                    response = sc.send_command("GB R");
                    RIGHT_beerName = response;
                }
                GB_feedback.Text = response;
            }
            catch (Exception ex) {
                GB_feedback.Text = ex.Message;
            }
        }

        private void BC_colorPicker_Click(object sender, RoutedEventArgs e) {
            System.Windows.Forms.ColorDialog colorDialog =
                       new System.Windows.Forms.ColorDialog();
            colorDialog.AllowFullOpen = true;
            colorDialog.ShowDialog();

            BC_red.Text = Convert.ToString(colorDialog.Color.R);
            BC_green.Text = Convert.ToString(colorDialog.Color.G);
            BC_blue.Text = Convert.ToString(colorDialog.Color.B);
        }

        private void TabControl_SelectionChanged(object sender, SelectionChangedEventArgs e) {
            if (BC.IsSelected) {
                Title.Text = "Set Backlight Color";
                e.Handled = true;
            }
            else if (SL.IsSelected) {
                Title.Text = "Set Keg Level (Beers)";
                e.Handled = true;
            }
            else if (SP.IsSelected) {
                Title.Text = "Set Keg Level (Pulses)";
                e.Handled = true;
            }
            else if (GP.IsSelected) {
                Title.Text = "Get Keg Level (Pulses)";
                e.Handled = true;
            }
            else if (SB.IsSelected) {
                Title.Text = "Set Beer Name";
                e.Handled = true;
            }
            else if (GB.IsSelected) {
                Title.Text = "Get Beer Name";
                e.Handled = true;
            }
        }

        private void PortRefresh_Click(object sender, RoutedEventArgs e) {
            ports = new List<string>(SerialPort.GetPortNames());
            Port.ItemsSource = ports;

            try {
                Port.SelectedIndex = 0;
            }
            catch (Exception ex) { }
        }

        private void BTN_saveState_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                sc.send_command("SV");
            }
            catch (Exception ex)
            {
            }
        }

        private void BTN_resetKegLeft_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                sc.send_command("RK L");
            }
            catch (Exception ex)
            {
            }
        }

        private void BTN_resetKegRight_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                sc.send_command("RK R");
            }
            catch (Exception ex)
            {
            }
        }

        private void BTN_pullState_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));
                
                // Get left pulses
                LEFT_sensorPulses = Convert.ToInt32(sc.send_command("GP L"));

                // Get right pulses
                RIGHT_sensorPulses = Convert.ToInt32(sc.send_command("GP R"));
                
                // Get left beer name
                LEFT_beerName = sc.send_command("GB L");

                // Get right beer name
                LEFT_beerName = sc.send_command("GB R");
            }
            catch (Exception ex)
            {
            }
        }

        private void BTN_pushState_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SerialCommunicator sc = new SerialCommunicator(Port.Text, Convert.ToInt32(((ComboBoxItem)Baud.SelectedItem).Content.ToString()));

                // Get left pulses
                sc.send_command("SP L " + format_pulse(LEFT_sensorPulses));

                // Get right pulses
                sc.send_command("SP R " + format_pulse(RIGHT_sensorPulses));

                // Get left beer name
                sc.send_command("SB L " + LEFT_beerName);

                // Get right beer name
                sc.send_command("SB R " + RIGHT_beerName);
            }
            catch (Exception ex)
            {
            }
        }
    }
}
