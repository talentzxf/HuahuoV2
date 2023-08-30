import logo from "./logo.svg";
import "./App.css";

function App() {
  return (
    <div className="flex h-full w-full flex-col items-center bg-main-bg">
      <div className="flex  w-[400px] flex-col items-center">
        <img
          src="https://eaassets-a.akamaihd.net/resource_signin_ea_com/551.0.20230817.661.974bc46/p/statics/juno/img/EALogo-New.svg"
          className="w-[64px]"
        ></img>
        <h1 className="mb-[24px] font-eaText text-[24px]  font-bold tracking-[.96px] text-white">
          Create your EA Account
        </h1>
        <div className="text-center font-eaDisplayText text-white">
          Follow a few steps to wrap up your EA Account(it's quick, we promise)
        </div>
      </div>
    </div>
  );
}

export default App;
