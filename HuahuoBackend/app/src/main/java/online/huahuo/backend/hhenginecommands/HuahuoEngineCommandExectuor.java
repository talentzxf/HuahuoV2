//package online.huahuo.backend.hhenginejni;
//
//import lombok.Data;
//import lombok.extern.java.Log;
//import org.springframework.beans.factory.InitializingBean;
//import org.springframework.beans.factory.annotation.Value;
//import org.springframework.stereotype.Service;
//
//import java.util.logging.Level;
//
//
//@Service
//@Log
//public class HuahuoEngineJNI implements InitializingBean {
//    @Value("${huahuo.engine.dllPath}")
//    String dllPath;
//
//    @Override
//    public void afterPropertiesSet() throws Exception {
//        try {
//            System.load("C:\\Users\\vincentzhang\\MyProjects\\HuahuoV2\\HuaHuoEngineV2\\cmake-build-debug\\libgcc_s_seh-1.dll");
//            System.load("C:\\Users\\vincentzhang\\MyProjects\\HuahuoV2\\HuaHuoEngineV2\\cmake-build-debug\\libstdc++-6.dll");
//            System.load(dllPath);
//        } catch (Exception e) {
//            log.log(Level.SEVERE, "Can't load dll:" + dllPath);
//            throw e;
//        }
//    }
//
//    private native ProjectFileMetaInfo getProjectMeta(String filePath);
//}
