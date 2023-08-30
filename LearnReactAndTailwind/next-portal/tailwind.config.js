/** @type {import('tailwindcss').Config} */
module.exports = {
  content: ["./src/**/*.{js,jsx,ts,tsx}"],
  theme: {
    extend: {
      'colors':{
        'main-bg': "#1d2033"
      },
      fontFamily:{
        eaText: ['Electronic Arts Text', 'sans-serif'],
        eaDisplayText: ['Electronic Arts Display', 'sans-serif']
      }
    },
  },
  plugins: [],
}

