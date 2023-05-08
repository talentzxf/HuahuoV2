package online.huahuo.backend.db;

import org.springframework.data.jpa.repository.JpaRepository;

public interface FileRepository extends JpaRepository<BinaryFileDB, Long> {
    BinaryFileDB findByCreatedByAndName(String createdBy, String fileName);
}
